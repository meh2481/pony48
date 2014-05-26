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
	m_hud->setScene("playing");
	
	setTimeScale(DEFAULT_TIMESCALE);
	
	//Game stuff!
	m_iCurMode = PLAYING;
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
}

void Pony48Engine::frame(float32 dt)
{
	soundUpdate(dt);
	
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
	}
}

void Pony48Engine::draw()
{
	glClearColor(m_BgCol.r, m_BgCol.g, m_BgCol.b, m_BgCol.a);
	//Set up camera and OpenGL flags
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	
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
	
	//Play music
	loadSongs("res/mus/music.xml");
	playMusic("res/mus/justfluttershy.mp3");
	musicLoop(23.259, 222.582);
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
				if(!(event.key.keysym.scancode == SDL_SCANCODE_W || 
					 event.key.keysym.scancode == SDL_SCANCODE_UP || 
					 event.key.keysym.scancode == SDL_SCANCODE_S ||
					 event.key.keysym.scancode == SDL_SCANCODE_DOWN ||
					 event.key.keysym.scancode == SDL_SCANCODE_A ||
					 event.key.keysym.scancode == SDL_SCANCODE_LEFT ||
					 event.key.keysym.scancode == SDL_SCANCODE_D ||
#ifdef DEBUG
					 event.key.keysym.scancode == SDL_SCANCODE_F5 ||
#endif
					 event.key.keysym.scancode == SDL_SCANCODE_RIGHT))
				{
					m_iCurMode = PLAYING;
					scrubResume();
					resetBoard();
					m_hud->setScene("playing");
					break;
				}
			}
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
					
				case SDL_SCANCODE_F5:
				{
					string sScene = m_hud->getScene();
					delete m_hud;
					m_hud = new HUD("hud");
					m_hud->create("res/hud/hud.xml");
					m_hud->setScene(sScene);
					break;
				}
				
				case SDL_SCANCODE_RETURN:
                    if(keyDown(SDL_SCANCODE_ALT))
						toggleFullscreen();
					break;
				
				case SDL_SCANCODE_W:
				case SDL_SCANCODE_UP:
					if(m_iCurMode == PLAYING)
					{
						bool moved = false;
						while(move(UP)) 
							moved = true;
						if(moved)
							placenew();
					}
					break;
					
				case SDL_SCANCODE_S:
				case SDL_SCANCODE_DOWN:
					if(m_iCurMode == PLAYING)
					{
						bool moved = false;
						while(move(DOWN)) 
							moved = true;
						if(moved)
							placenew();
					}
					break;
					
				case SDL_SCANCODE_A:
				case SDL_SCANCODE_LEFT:
					if(m_iCurMode == PLAYING)
					{
						bool moved = false;
						while(move(LEFT)) 
							moved = true;
						if(moved)
							placenew();
					}
					break;
					
				case SDL_SCANCODE_D:
				case SDL_SCANCODE_RIGHT:
					if(m_iCurMode == PLAYING)
					{
						bool moved = false;
						while(move(RIGHT)) 
							moved = true;
						if(moved)
							placenew();
					}
					break;
			}
			break;
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
	if(m_iCurMode == PLAYING)
	{
#ifdef DEBUG
		if(keyDown(SDL_SCANCODE_B))
			setTimeScale(DEFAULT_TIMESCALE/3);
		else
			setTimeScale(DEFAULT_TIMESCALE);
#endif
		if(keyDown(SDL_SCANCODE_W) || keyDown(SDL_SCANCODE_UP))
		{
			m_BoardRot.x = 1;
			m_BoardRot.y = 0;
			if(m_BoardRotAngle > -MAX_BOARD_ROT_ANGLE)
				m_BoardRotAngle -= BOARD_ROT_AMT;
		}	
		else if(keyDown(SDL_SCANCODE_S) || keyDown(SDL_SCANCODE_DOWN))
		{
			m_BoardRot.x = 1;
			m_BoardRot.y = 0;
			if(m_BoardRotAngle < MAX_BOARD_ROT_ANGLE)
				m_BoardRotAngle += BOARD_ROT_AMT;
		}
		else if(keyDown(SDL_SCANCODE_A) || keyDown(SDL_SCANCODE_LEFT))
		{
			m_BoardRot.x = 0;
			m_BoardRot.y = 1;
			if(m_BoardRotAngle > -MAX_BOARD_ROT_ANGLE)
				m_BoardRotAngle -= BOARD_ROT_AMT;
		}
		else if(keyDown(SDL_SCANCODE_D) || keyDown(SDL_SCANCODE_RIGHT))
		{
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
	
}


































