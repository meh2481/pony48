/*
	Pony48 source - Pony48.cpp
	Copyright (c) 2014 Mark Hutcheson
*/

#include "Pony48.h"
#include "tinyxml2.h"
#include <float.h>
#include <sstream>
#include <iomanip>

//For our engine functions to be able to call our Engine class functions
static Pony48Engine* g_pGlobalEngine;

void signalHandler(string sSignal)
{
	g_pGlobalEngine->hudSignalHandler(sSignal);
}

Pony48Engine::Pony48Engine(uint16_t iWidth, uint16_t iHeight, string sTitle, string sAppName, string sIcon, bool bResizable) : 
Engine(iWidth, iHeight, sTitle, sAppName, sIcon, bResizable)
{
	g_pGlobalEngine = this;
	vfs.Prepare();
	
	//Set camera position for this game
	m_fDefCameraZ = -16;
	CameraPos.set(0,0,m_fDefCameraZ);
	m_BoardRotAngle = 0;
#ifdef DEBUG
	m_bMouseGrabOnWindowRegain = false;
#else
	m_bMouseGrabOnWindowRegain = true;
#endif	
	showCursor();
	
	m_hud = new HUD("hud");
	m_hud->create("res/hud/hud.xml");
	m_hud->setScene("intro");
	
	setTimeScale(DEFAULT_TIMESCALE);
	
	m_joy = NULL;
	m_VideoCap = new cv::VideoCapture(0);
	if(!m_VideoCap->isOpened())
		errlog << "Unable to open webcam." << endl;
	
	//Game stuff!
	m_iCurMode = INTRO;//PLAYING;
	m_fGameoverKeyDelay = 0;
	m_BoardBg.set(0.7,0.7,0.7,1);
	m_TileBg.set(0.5,0.5,0.5,1);
	m_BgCol.set(0,0,0,1.0);
	m_iHighScore = 0;
	starfieldBg* bg = new starfieldBg();
	bg->init();
	m_bg = (Background*) bg;
	m_BoardBg.a = 0.2;
	m_TileBg.a = 0.2;
}

Pony48Engine::~Pony48Engine()
{
	errlog << "~Pony48Engine()" << endl;
	saveConfig(getSaveLocation() + "config.xml");
	clearBoard();
	if(m_bg != NULL)
		delete m_bg;
	errlog << "delete hud" << endl;
	delete m_hud;
	if(SDL_JoystickGetAttached(m_joy))
		SDL_JoystickClose(m_joy);
	delete m_VideoCap;
}

void Pony48Engine::frame(float32 dt)
{
	switch(m_iCurMode)
	{
		case PLAYING:
		case GAMEOVER:
			soundUpdate(dt);
			updateBoard(dt);
			
			//First half of camera bounce; move back a bit every frame in an attempt to get back to default position
			if(CameraPos.z > m_fDefCameraZ)
				CameraPos.z -= 0.3;
			if(CameraPos.z < m_fDefCameraZ)
				CameraPos.z = m_fDefCameraZ;
			
			//Second half of camera bounce; move forward on every bass kick
			beatDetect();
			
			//Handle key presses
			handleKeys();
			
			//Update background
			if(m_bg != NULL)
				m_bg->update(dt);
			
			//Check if game is now over
			if(m_iCurMode == PLAYING && !movePossible())
			{
				//Update final score counter
				ostringstream oss;
				HUDTextbox* txt = (HUDTextbox*)m_hud->getChild("finalscore");
				oss << "FINAL SCORE: " << m_iScore;
				txt->setText(oss.str());
				m_hud->setScene("gameover");
				m_iCurMode = GAMEOVER;
				scrubPause();
				m_fGameoverKeyDelay = getSeconds();
			}
			break;
		
		case INTRO:
		{
			//Set icon if user has/doesn't have gamepad
			HUDItem* hIt = m_hud->getChild("yespad");
			if(hIt != NULL)
				hIt->hidden = (m_joy == NULL);
			hIt = m_hud->getChild("nopad");
			if(hIt != NULL)
				hIt->hidden = (m_joy != NULL);
				
			//Set icon if user has/doesn't have webcam
			hIt = m_hud->getChild("yescam");
			if(hIt != NULL)
				hIt->hidden = !m_VideoCap->isOpened();
			hIt = m_hud->getChild("nocam");
			if(hIt != NULL)
				hIt->hidden = m_VideoCap->isOpened();
			
			//Calculate the proper alpha value for the black cover-up-intro graphic for our fadein time
			float alpha = (getSeconds()-INTRO_FADEIN_DELAY) / INTRO_FADEIN_TIME;
			if(alpha < 0) alpha = 0;
			if(alpha > 1) alpha = 1;
			hIt = m_hud->getChild("coverintro");
			if(hIt != NULL)
				hIt->col.a = 1.0-alpha;
			break;
		}
	}
}

void Pony48Engine::draw()
{
	glClearColor(m_BgCol.r, m_BgCol.g, m_BgCol.b, m_BgCol.a);
	//Set up camera and OpenGL flags
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	
	switch(m_iCurMode)
	{
		case PLAYING:
		case GAMEOVER:
		{
			//Draw background behind everything else
			if(m_bg != NULL)
			{
				glLoadIdentity();
				glTranslatef(0, 0, m_fDefCameraZ);
				Rect rcView = getCameraView();
				m_bg->screenDiag = sqrt(rcView.width()*rcView.width()+rcView.height()*rcView.height());	//HACK: Update every frame to handle screen resize
				m_bg->draw();
				glClear(GL_DEPTH_BUFFER_BIT);
			}
			
			//Set up OpenGL matrices
			glLoadIdentity();
			glTranslatef(CameraPos.x, CameraPos.y, CameraPos.z);
			glRotatef(m_BoardRotAngle, m_BoardRot.x, m_BoardRot.y, m_BoardRot.z);
			
			//Draw our game info
			drawBoard();
			drawObjects();
			
			//Update HUD score
			HUDTextbox* txt = (HUDTextbox*)m_hud->getChild("scorebox");
			ostringstream oss;
			oss << "SCORE: " << m_iScore;
			txt->setText(oss.str());
			oss.str("");
			txt = (HUDTextbox*)m_hud->getChild("hiscorebox");
			oss << "BEST: " << m_iHighScore;
			txt->setText(oss.str());
		}
		break;
	}
	
	//Draw HUD always at this depth, on top of everything else
	glClear(GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0, 0, m_fDefCameraZ);
	m_hud->draw(0);
}

void Pony48Engine::init(list<commandlineArg> sArgs)
{
	//Run through list for arguments we recognize
	for(list<commandlineArg>::iterator i = sArgs.begin(); i != sArgs.end(); i++)
	{
		errlog << "Commandline argument. Switch: " << i->sSwitch << ", value: " << i->sValue << endl;
	}
	
	//Load our last screen position and such
	loadConfig(getSaveLocation() + "config.xml");
	
	//Set gravity to 0
	getWorld()->SetGravity(b2Vec2(0,0));
	
	//Init board
	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
			m_Board[j][i] = NULL;
	}
	resetBoard();
	
	//Create sounds up front
	//createSound("res/sfx/select.ogg", "select");			//When you're selecting different menu items
	
	//pauseMusic();
	hideCursor();
}


void Pony48Engine::hudSignalHandler(string sSignal)
{
}

void Pony48Engine::handleEvent(SDL_Event event)
{
	m_hud->event(event);	//Let our HUD handle any events it needs to
	switch(event.type)
	{
		//Key pressed
		case SDL_KEYDOWN:
		{
			if(m_iCurMode == GAMEOVER)
			{
				if(!(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE
#ifdef DEBUG
					 || event.key.keysym.scancode == SDL_SCANCODE_F5
#endif
					 ) && getSeconds() - m_fGameoverKeyDelay >= GAMEOVER_KEY_DELAY)	//Wait for a certain amount of time, in case they just mashed keys accidentally
				{
					m_iCurMode = PLAYING;
					scrubResume();
					resetBoard();
					m_hud->setScene("playing");
					break;
				}
			}
			else if(m_iCurMode == PLAYING)
			{
				switch(event.key.keysym.scancode)
				{
					case SDL_SCANCODE_ESCAPE:
						quit();
						break;
					
					case SDL_SCANCODE_F10:
					case SDL_SCANCODE_G:
						if(keyDown(SDL_SCANCODE_CTRL))
						{
							grabMouse(!isMouseGrabbed());	//Toggle grabbing/releasing the mouse
						}
						break;
#ifdef DEBUG
					case SDL_SCANCODE_F5:
					{
						string sScene = m_hud->getScene();
						delete m_hud;
						m_hud = new HUD("hud");
						m_hud->create("res/hud/hud.xml");
						m_hud->setScene(sScene);
						break;
					}
#endif
					case SDL_SCANCODE_RETURN:
						if(keyDown(SDL_SCANCODE_ALT))
							toggleFullscreen();
						break;
					
					case SDL_SCANCODE_W:
					case SDL_SCANCODE_UP:
						if(m_iCurMode == PLAYING)
							move(UP);
						break;
						
					case SDL_SCANCODE_S:
					case SDL_SCANCODE_DOWN:
						if(m_iCurMode == PLAYING)
							move(DOWN);
						break;
						
					case SDL_SCANCODE_A:
					case SDL_SCANCODE_LEFT:
						if(m_iCurMode == PLAYING)
							move(LEFT);
						break;
						
					case SDL_SCANCODE_D:
					case SDL_SCANCODE_RIGHT:
						if(m_iCurMode == PLAYING)
							move(RIGHT);
						break;
				}
				break;
			}
			else if(m_iCurMode == INTRO)
			{
				if(event.key.keysym.scancode == SDL_SCANCODE_SPACE)
				{
					//Re-test joystick
					SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);
					m_joy = NULL;
					SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);
					//Re-test webcam
					delete m_VideoCap;
					m_VideoCap = new cv::VideoCapture(0);
				}
#ifdef DEBUG
				else if(event.key.keysym.scancode == SDL_SCANCODE_F5)
				{
					string sScene = m_hud->getScene();
					delete m_hud;
					m_hud = new HUD("hud");
					m_hud->create("res/hud/hud.xml");
					m_hud->setScene(sScene);
				}
#endif
				else
				{
					m_iCurMode = PLAYING;
					m_hud->setScene("playing");
					//Play music
					loadSongs("res/mus/music.xml");
				}
			}
		}
		
		//Key released
		case SDL_KEYUP:
			switch(event.key.keysym.scancode)
			{
			}
			break;
		
		case SDL_MOUSEBUTTONDOWN:
			if(event.button.button == SDL_BUTTON_LEFT)
			{
			}
			else if(event.button.button == SDL_BUTTON_RIGHT)
			{
			}
			else if(event.button.button == SDL_BUTTON_MIDDLE)
			{
			}
			break;
			
		case SDL_MOUSEWHEEL:
			/*if(event.wheel.y > 0)
			{
				CameraPos.z = min(CameraPos.z + 1.5, -5.0);
			}
			else
			{
				CameraPos.z = max(CameraPos.z - 1.5, -3000.0);
			}
			cameraBounds();*/
			break;

		case SDL_MOUSEBUTTONUP:
			if(event.button.button == SDL_BUTTON_LEFT)
			{
				
			}
			else if(event.button.button == SDL_BUTTON_RIGHT)
			{
			
			}
			else if(event.button.button == SDL_BUTTON_MIDDLE)
			{
				
			}
			break;

		case SDL_MOUSEMOTION:
			break;
		
		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
				case SDL_WINDOWEVENT_FOCUS_LOST:
					m_bMouseGrabOnWindowRegain = isMouseGrabbed();
					grabMouse(false);	//Ungrab mouse cursor if alt-tabbing out or such
					break;
				
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					grabMouse(m_bMouseGrabOnWindowRegain);	//Grab mouse on input regain, if we should
					break;
			}
			break;
			
		//Gamepad stuff!
		case SDL_JOYDEVICEADDED:
			errlog << "Joystick " << (int)event.jdevice.which << " connected." << endl;
			m_joy = SDL_JoystickOpen(event.jdevice.which);

			if(m_joy) 
			{
				errlog << "Opened Joystick " << event.jdevice.which << endl;
				errlog << "Name: " << SDL_JoystickNameForIndex(event.jdevice.which) << endl;
				errlog << "Number of Axes: " << SDL_JoystickNumAxes(m_joy) << endl;
				errlog << "Number of Buttons: " << SDL_JoystickNumButtons(m_joy) << endl;
				errlog << "Number of Balls: " << SDL_JoystickNumBalls(m_joy) << endl;
				errlog << "Number of Hats: " << SDL_JoystickNumHats(m_joy) << endl;
			} 
			else
				errlog << "Couldn't open Joystick " << (int)event.jdevice.which << endl;
			
			/*if (SDL_IsGameController(event.jdevice.which)) 
			{
				m_controller = SDL_GameControllerOpen(event.jdevice.which);
				if (m_controller)
				{
					cout << "Opened Controller " << event.jdevice.which << endl;
				}
				else
					cout << "Could not open gamecontroller: " << SDL_GetError() << endl;
			}*/

			break;
			
		case SDL_JOYDEVICEREMOVED:
			errlog << "Joystick " << (int)event.jdevice.which << " disconnected." << endl;
			break;
			
		case SDL_JOYBUTTONDOWN:
			//cout << "Joystick " << (int)event.jbutton.which << " pressed button " << (int)event.jbutton.button << endl;
			if(m_iCurMode == GAMEOVER)
			{
				m_iCurMode = PLAYING;
				scrubResume();
				resetBoard();
				m_hud->setScene("playing");
			}
			else if(m_iCurMode == INTRO)
			{
				m_iCurMode = PLAYING;
				m_hud->setScene("playing");
				loadSongs("res/mus/music.xml");
			}
			break;
			
		case SDL_JOYBUTTONUP:
			//cout << "Joystick " << (int)event.jbutton.which << " released button " << (int)event.jbutton.button << endl;
			break;
			
		case SDL_JOYAXISMOTION:
			//TODO: Deal with range of axis movement; binary on/off doesn't work with analog sticks
			if(m_iCurMode == GAMEOVER && getSeconds() - m_fGameoverKeyDelay >= GAMEOVER_KEY_DELAY)
			{
				m_iCurMode = PLAYING;
				scrubResume();
				resetBoard();
				m_hud->setScene("playing");
				break;
			}
			else if(m_iCurMode == PLAYING && event.jaxis.axis == 0)	//Horizontal axis
			{
				if(event.jaxis.value < -JOY_AXIS_TRIP)	//Left
				{
					if(m_iCurMode == PLAYING)
						move(LEFT);
				}
				else if(event.jaxis.value > JOY_AXIS_TRIP)	//Right
				{
					if(m_iCurMode == PLAYING)
						move(RIGHT);
				}
			}
			else if(event.jaxis.axis == 1)	//Vertical axis
			{
				if(event.jaxis.value < -JOY_AXIS_TRIP)	//Up
				{
					if(m_iCurMode == PLAYING)
						move(UP);
				}
				else if(event.jaxis.value > JOY_AXIS_TRIP)	//Down
				{
					if(m_iCurMode == PLAYING)
						move(DOWN);
				}
			}
			break;
	}
}

void Pony48Engine::pause()
{
	pauseMusic();
}

void Pony48Engine::resume()
{
	resumeMusic();
}

Rect Pony48Engine::getCameraView()
{
	Rect rcCamera;
	const float32 tan45_2 = tan(DEG2RAD*45/2);
	const float32 fAspect = (float32)getWidth() / (float32)getHeight();
	rcCamera.bottom = (tan45_2 * CameraPos.z);
	rcCamera.top = -(tan45_2 * CameraPos.z);
	rcCamera.left = rcCamera.bottom * fAspect;
	rcCamera.right = rcCamera.top * fAspect;
	rcCamera.offset(CameraPos.x, CameraPos.y);
	return rcCamera;
}

Point Pony48Engine::worldMovement(Point cursormove)
{
	cursormove.x /= (float32)getWidth();
	cursormove.y /= (float32)getHeight();
	
	Rect rcCamera = getCameraView();
	cursormove.x *= rcCamera.width();
	cursormove.y *= -rcCamera.height();	//Flip y
	
	return cursormove;
}

Point Pony48Engine::worldPosFromCursor(Point cursorpos)
{
	//Rectangle that the camera can see in world space
	Rect rcCamera = getCameraView();
	
	//Our relative position in window rect space (in rage 0-1)
	cursorpos.x /= (float32)getWidth();
	cursorpos.y /= (float32)getHeight();
	
	//Multiply this by the size of the world rect to get the relative cursor pos
	cursorpos.x = cursorpos.x * rcCamera.width() + rcCamera.left;
	cursorpos.y = cursorpos.y * rcCamera.height() + rcCamera.top;	//Flip on y axis
	
	return cursorpos;
}

void Pony48Engine::loadConfig(string sFilename)
{
	errlog << "Parsing config file " << sFilename << endl;
	//Open file
	XMLDocument* doc = new XMLDocument;
	int iErr = doc->LoadFile(sFilename.c_str());
	if(iErr != XML_NO_ERROR)
	{
		errlog << "Error parsing config file: Error " << iErr << ". Ignoring..." << endl;
		if(isFullscreen())
			setInitialFullscreen();
		delete doc;
		return;	//No file; ignore
	}
	
	//Grab root element
	XMLElement* root = doc->RootElement();
	if(root == NULL)
	{
		errlog << "Error: Root element NULL in XML file. Ignoring..." << endl;
		if(isFullscreen())
			setInitialFullscreen();
		delete doc;
		return;
	}
	
	XMLElement* window = root->FirstChildElement("window");
	if(window != NULL)
	{
		bool bFullscreen = isFullscreen();
		bool bMaximized = isMaximized();
		uint32_t width = getWidth();
		uint32_t height = getHeight();
		uint32_t framerate = getFramerate();
		bool bDoubleBuf = getDoubleBuffered();
		bool bPausesOnFocus = pausesOnFocusLost();
		int iVsync = getVsync();
		int iMSAA = getMSAA();
		float32 fGamma = getGamma();
		
		window->QueryUnsignedAttribute("width", &width);
		window->QueryUnsignedAttribute("height", &height);
		window->QueryBoolAttribute("fullscreen", &bFullscreen);
		window->QueryBoolAttribute("maximized", &bMaximized);
		window->QueryUnsignedAttribute("fps", &framerate);
		window->QueryBoolAttribute("doublebuf", &bDoubleBuf);
		window->QueryIntAttribute("vsync", &iVsync);
		window->QueryIntAttribute("MSAA", &iMSAA);
		window->QueryFloatAttribute("brightness", &fGamma);
		window->QueryBoolAttribute("pauseminimized", &bPausesOnFocus);
		
		const char* cWindowPos = window->Attribute("pos");
		if(cWindowPos != NULL)
		{
			Point pos = pointFromString(cWindowPos);
			setWindowPos(pos);
		}
		setFullscreen(bFullscreen);
		changeScreenResolution(width, height);
		if(bMaximized && !isMaximized() && !bFullscreen)
			maximizeWindow();
		setFramerate(framerate);
		setVsync(iVsync);
		setDoubleBuffered(bDoubleBuf);
		setMSAA(iMSAA);
		setGamma(fGamma);
		pauseOnKeyboard(bPausesOnFocus);
	}
	
	XMLElement* pony48 = root->FirstChildElement("pony48");
	if(pony48 != NULL)
	{
		pony48->QueryUnsignedAttribute("highscore", &m_iHighScore);
	}
	
	delete doc;
}

void Pony48Engine::saveConfig(string sFilename)
{
	errlog << "Saving config XML " << sFilename << endl;
	XMLDocument* doc = new XMLDocument;
	XMLElement* root = doc->NewElement("config");
	
	XMLElement* window = doc->NewElement("window");
	window->SetAttribute("width", getWidth());
	window->SetAttribute("height", getHeight());
	window->SetAttribute("fullscreen", isFullscreen());
	window->SetAttribute("maximized", isMaximized());
	window->SetAttribute("pos", pointToString(getWindowPos()).c_str());
	window->SetAttribute("fps", (uint32_t)(getFramerate()));
	window->SetAttribute("vsync", getVsync());
	window->SetAttribute("doublebuf", getDoubleBuffered());
	window->SetAttribute("MSAA", getMSAA());
	window->SetAttribute("brightness", getGamma());
	window->SetAttribute("pauseminimized", pausesOnFocusLost());
	root->InsertEndChild(window);
	
	XMLElement* pony48 = doc->NewElement("pony48");
	pony48->SetAttribute("highscore", m_iHighScore);
	root->InsertEndChild(pony48);
	
	doc->InsertFirstChild(root);
	doc->SaveFile(sFilename.c_str());
	delete doc;
}

obj* Pony48Engine::objFromXML(string sXMLFilename, Point ptOffset, Point ptVel)
{
	return NULL;
}

#define BOARD_ROT_AMT	5
#define MAX_BOARD_ROT_ANGLE	25

void Pony48Engine::handleKeys()
{
#ifdef DEBUG
	if(keyDown(SDL_SCANCODE_B))
		setTimeScale(DEFAULT_TIMESCALE/3);
	else
		setTimeScale(DEFAULT_TIMESCALE);
#endif
	Sint16 x_move = 0;
	Sint16 y_move = 0;
	if(SDL_JoystickGetAttached(m_joy))
	{
		x_move = SDL_JoystickGetAxis(m_joy, 0);
		y_move = SDL_JoystickGetAxis(m_joy, 1);
	}
	Point vecMove((float32)x_move/(float32)JOY_AXIS_MAX, (float32)y_move/(float32)JOY_AXIS_MAX);
	
	if(keyDown(SDL_SCANCODE_W) || keyDown(SDL_SCANCODE_UP) || y_move < -JOY_AXIS_TRIP)
	{
		vecMove.y += 1;
		m_BoardRot.x = 1;
		m_BoardRot.y = 0;
		if(m_BoardRotAngle > -MAX_BOARD_ROT_ANGLE)
			m_BoardRotAngle -= BOARD_ROT_AMT;
	}	
	else if(keyDown(SDL_SCANCODE_S) || keyDown(SDL_SCANCODE_DOWN) || y_move > JOY_AXIS_TRIP)
	{
		vecMove.y -= 1;
		m_BoardRot.x = 1;
		m_BoardRot.y = 0;
		if(m_BoardRotAngle < MAX_BOARD_ROT_ANGLE)
			m_BoardRotAngle += BOARD_ROT_AMT;
	}
	else if(keyDown(SDL_SCANCODE_A) || keyDown(SDL_SCANCODE_LEFT) || x_move < -JOY_AXIS_TRIP)
	{		
		vecMove.x -= 1;
		m_BoardRot.x = 0;
		m_BoardRot.y = 1;
		if(m_BoardRotAngle > -MAX_BOARD_ROT_ANGLE)
			m_BoardRotAngle -= BOARD_ROT_AMT;
	}
	else if(keyDown(SDL_SCANCODE_D) || keyDown(SDL_SCANCODE_RIGHT) || x_move > JOY_AXIS_TRIP)
	{
		vecMove.x += 1;
		m_BoardRot.x = 0;
		m_BoardRot.y = 1;
		if(m_BoardRotAngle < MAX_BOARD_ROT_ANGLE)
			m_BoardRotAngle += BOARD_ROT_AMT;
	}
	else if(m_BoardRotAngle < 0)
	{
		m_BoardRotAngle += BOARD_ROT_AMT;
		if(m_BoardRotAngle > 0)
		{
			m_BoardRotAngle = 0;
			m_BoardRot.x = m_BoardRot.y = 0;
		}
	}
	else if(m_BoardRotAngle > 0)
	{
		m_BoardRotAngle -= BOARD_ROT_AMT;
		if(m_BoardRotAngle < 0)
		{
			m_BoardRotAngle = 0;
			m_BoardRot.x = m_BoardRot.y = 0;
		}
	}
	
}


































