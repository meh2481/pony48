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
	m_fDefCameraZ = -15;
	CameraPos.x = 0;
	CameraPos.y = 0;
	CameraPos.z = m_fDefCameraZ;
#ifdef DEBUG
	m_bMouseGrabOnWindowRegain = false;
#else
	m_bMouseGrabOnWindowRegain = true;
#endif	
	showCursor();
	
	//m_hud = new HUD("hud");
	//m_hud->create("res/hud.xml");
	//m_hud->setScene("start");
	
	setTimeScale(DEFAULT_TIMESCALE);
}

Pony48Engine::~Pony48Engine()
{
	errlog << "~Pony48Engine()" << endl;
	saveConfig(getSaveLocation() + "config.xml");
	//Delete stuffs
	errlog << "delete hud" << endl;
	//delete m_hud;
}

void Pony48Engine::frame(float32 dt)
{
	if(CameraPos.z > m_fDefCameraZ)
		CameraPos.z -= 0.3;
	if(CameraPos.z < m_fDefCameraZ)
		CameraPos.z = m_fDefCameraZ;
	beatDetect();
}

void Pony48Engine::draw()
{
	//Set up camera and OpenGL flags
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glLoadIdentity();
	
	glTranslatef(CameraPos.x, CameraPos.y, CameraPos.z);
	
	
	drawObjects();
	
	
	//Draw HUD always at this depth, on top of everything else
	glClear(GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0, 0, m_fDefCameraZ);
	//m_hud->draw(0);
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
	
	physSegment* testseg = new physSegment;
	testseg->img = getImage("res/gfx/ab/1.png");
	testseg->size = Point(2,2);
	addScenery(testseg);
	
	//Create sounds up front
	//createSound("res/sfx/select.ogg", "select");			//When you're selecting different menu items
	
	//Play music
	playMusic("res/mus/justfluttershy.mp3");
	//pauseMusic();
	hideCursor();
}


void Pony48Engine::hudSignalHandler(string sSignal)
{
}

void Pony48Engine::handleEvent(SDL_Event event)
{
	//m_hud->event(event);	//Let our HUD handle any events it needs to
	switch(event.type)
	{
		//Key pressed
		case SDL_KEYDOWN:
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
					//string sScene = m_hud->getScene();
					//delete m_hud;
					//m_hud = new HUD("hud");
					//m_hud->create("res/hud.xml");
					//m_hud->setScene(sScene);
					//break;
				}
			}
			break;

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
	
	doc->InsertFirstChild(root);
	doc->SaveFile(sFilename.c_str());
	delete doc;
}

obj* Pony48Engine::objFromXML(string sXMLFilename, Point ptOffset, Point ptVel)
{
	return NULL;
}

void Pony48Engine::handleKeys()
{
#ifdef DEBUG
	if(keyDown(SDL_SCANCODE_B))
		setTimeScale(DEFAULT_TIMESCALE/3);
	else
		setTimeScale(DEFAULT_TIMESCALE);
#endif
}


