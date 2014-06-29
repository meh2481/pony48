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
Pony48Engine* g_pGlobalEngine;

//Keybinding stuff!
uint32_t JOY_BUTTON_BACK;
uint32_t JOY_BUTTON_START;
uint32_t JOY_BUTTON_X;
uint32_t JOY_BUTTON_Y;
uint32_t JOY_BUTTON_A;
uint32_t JOY_BUTTON_B;
uint32_t JOY_BUTTON_LB;
uint32_t JOY_BUTTON_RB;
uint32_t JOY_BUTTON_LSTICK;
uint32_t JOY_BUTTON_RSTICK;
uint32_t JOY_AXIS_HORIZ;
uint32_t JOY_AXIS_VERT;
uint32_t JOY_AXIS2_HORIZ;
uint32_t JOY_AXIS2_VERT;
uint32_t JOY_AXIS_LT;
uint32_t JOY_AXIS_RT;
int32_t JOY_AXIS_TRIP;
SDL_Scancode KEY_UP1;
SDL_Scancode KEY_UP2;
SDL_Scancode KEY_DOWN1;
SDL_Scancode KEY_DOWN2;
SDL_Scancode KEY_LEFT1;
SDL_Scancode KEY_LEFT2;
SDL_Scancode KEY_RIGHT1;
SDL_Scancode KEY_RIGHT2;
SDL_Scancode KEY_ENTER1;
SDL_Scancode KEY_ENTER2;

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
	hideCursor();
	m_mCursors["sel"] = new myCursor();
	m_mCursors["sel"]->fromXML("res/cursor/arrowsel.xml");
	setCursor(m_mCursors["sel"]);
	m_mCursors["dir"] = new myCursor();
	m_mCursors["dir"]->fromXML("res/cursor/arrowdir.xml");
	m_iMouseControl = 0;
	
	m_hud = new HUD("hud");
	m_hud->create("res/hud.xml");
	m_hud->setScene("intro");
	m_hud->setSignalHandler(signalHandler);
	
	HUDItem* hIt = m_hud->getChild("songmenu");
	if(hIt != NULL)
	{
		HUDMenu* hMen = (HUDMenu*)hIt;
		phaseColor(&hMen->m_sSelected, hMen->m_sSelected2, 0.13f, true);
	}
	
	setTimeScale(DEFAULT_TIMESCALE);
	
	m_joy = NULL;
	m_rumble = NULL;
	m_cam = new Webcam;
	
	//Game stuff!
	m_iCurMode = INTRO;
	INTRO_FADEIN_DELAY = 10.0;	//temporarily set this until init()
	m_fGameoverKeyDelay = 0;
	m_BoardBg.set(0.7,0.7,0.7,.5);
	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
			m_TileBg[j][i].set(0.5,0.5,0.5,.5);
	}
	m_BgCol.set(0,0,0,1.0);
	m_iHighScore = 0;
	m_bg = NULL;
	bJoyVerticalMove = bJoyHorizontalMove = false;
	
	m_fLastFrame = 0.0f;
	
	//Keybinding stuff!
	JOY_AXIS_HORIZ = 0;
	JOY_AXIS_VERT = 1;
	JOY_AXIS_RT = 5;
	JOY_AXIS_TRIP = 20000;
	m_lastJoyHatMoved = 0;
	KEY_UP1 = SDL_SCANCODE_W;
	KEY_UP2 = SDL_SCANCODE_UP;
	KEY_DOWN1 = SDL_SCANCODE_S;
	KEY_DOWN2 = SDL_SCANCODE_DOWN;
	KEY_LEFT1 = SDL_SCANCODE_A;
	KEY_LEFT2 = SDL_SCANCODE_LEFT;
	KEY_RIGHT1 = SDL_SCANCODE_D;
	KEY_RIGHT2 = SDL_SCANCODE_RIGHT;
	KEY_ENTER1 = SDL_SCANCODE_SPACE;
	KEY_ENTER2 = SDL_SCANCODE_RETURN;
	
	//Apparently our Xbox drivers for different OS's can't agree on which buttons are which
#ifdef _WIN32
	JOY_BUTTON_BACK = 5;
	JOY_BUTTON_START = 4;	//TODO
	JOY_BUTTON_X = 12;
	JOY_BUTTON_Y = 13;
	JOY_BUTTON_A = 10;
	JOY_BUTTON_B = 11;
	JOY_BUTTON_LB = 8;
	JOY_BUTTON_RB = 9;
	JOY_BUTTON_LSTICK = 6;
	JOY_BUTTON_RSTICK = 7;
	JOY_AXIS2_HORIZ = 2;
	JOY_AXIS2_VERT = 3;
	JOY_AXIS_LT = 4;
#else
	JOY_BUTTON_BACK = 6;
	JOY_BUTTON_START = 7;
	JOY_BUTTON_X = 2;
	JOY_BUTTON_Y = 3;
	JOY_BUTTON_A = 0;
	JOY_BUTTON_B = 1;
	JOY_BUTTON_LB = 4;
	JOY_BUTTON_RB = 5;
	JOY_BUTTON_LSTICK = 9;
	JOY_BUTTON_RSTICK = 10;
	JOY_AXIS2_HORIZ = 3;
	JOY_AXIS2_VERT = 4;
	JOY_AXIS_LT = 2;
#endif
	
	//Camera stuff!
	m_iCAM = 0;
#ifndef USE_VIDEOINPUT
	m_iCAM_FRAME_SKIP = 7;
	m_iCurCamFrameSkip = 0;
#endif
	m_fGameoverWebcamFreeze = 0;
	m_fWebcamDrawSize = 4;
	m_ptWebcamDrawPos.Set(-6.2,5);
	m_bDrawFacecam = false;
	
	//Other stuff!
	m_fArrowAdd = 0;
	m_bJoyControl = false;
	m_fMusicVolume = 0.5f;
	m_fMusicFadeInVolume = 0.0f;
	m_fMusicScrubSpeed = soundFreqDefault;
	m_fSoundVolume = 1.0f;
	m_fVoxVolume = 1.0f;
	m_bHasBoredVox = false;
	m_fLastMovedSec = 0.0f;
	m_fSongFxRotate = 0.0f;
	m_selectedSongArc = new arc(64, getImage("res/particles/rainbowblur.png"));
	m_selectedSongArc->add = 0.4;
	m_selectedSongArc->max = 0.4;
	m_selectedSongArc->height = 0.2;
	m_selectedSongArc->avg = 1;
	m_selectedSongArc->init();
	m_fFadeoutTitleTime = getSeconds();
	m_bSavedFacepic = false;
	
	m_gameoverTileRot = 0;	//Happy now, Valgrind?
	m_gameoverTileVel = 30;
	m_gameoverTileAccel = 16;
}

Pony48Engine::~Pony48Engine()
{
	errlog << "~Pony48Engine()" << endl;
	saveConfig(getSaveLocation() + "config.xml");
	clearBoard();	
	clearColors();
	cleanupSongGfx();
	if(m_bg != NULL)
		delete m_bg;
	for(map<string, myCursor*>::iterator i = m_mCursors.begin(); i != m_mCursors.end(); i++)
		delete i->second;
	for(map<string, ParticleSystem*>::iterator i = m_ScoreParticles.begin(); i != m_ScoreParticles.end(); i++)
		delete i->second;
	for(vector<ParticleSystem*>::iterator i = m_selectedSongParticles.begin(); i != m_selectedSongParticles.end(); i++)
		delete *i;
	for(list<ParticleSystem*>::iterator i = m_selectedSongParticlesBg.begin(); i != m_selectedSongParticlesBg.end(); i++)
		delete *i;
	errlog << "delete hud" << endl;
	delete m_hud;
	if(m_rumble != NULL)
		SDL_HapticClose(m_rumble);
	if(SDL_JoystickGetAttached(m_joy))
		SDL_JoystickClose(m_joy);
	delete m_selectedSongArc;
	delete m_cam;
}

const float32 MUSIC_SCRUBIN_SPEED = soundFreqDefault * 2.0f;

void Pony48Engine::frame(float32 dt)
{
#ifdef DEBUG
	if(m_joy != NULL && SDL_JoystickGetButton(m_joy, 4))	//Slooow waaay dooown so we can see if everything's working properly
		dt /= 64.0;
#endif
	switch(m_iCurMode)
	{
		case PLAYING:
			if(getSeconds() - m_fLastMovedSec > BORED_VOX_TIME && !m_bHasBoredVox)
			{
				m_bHasBoredVox = true;
				playSound("nowhacking_theyreponies", m_fVoxVolume);
			}
		case GAMEOVER:
			m_gameoverTileRot += m_gameoverTileVel * dt;
			m_gameoverTileVel += ((m_gameoverTileRot > 0)?(-m_gameoverTileAccel):(m_gameoverTileAccel)) * dt;
			
			soundUpdate(dt);
			updateBoard(dt);
			for(map<string, ParticleSystem*>::iterator i = m_ScoreParticles.begin(); i != m_ScoreParticles.end(); i++)
				i->second->update(dt);
			
			//Bounce camera forward on every bass kick
			beatDetect();
			
			//Handle key presses
			handleKeys();
			
			//Check if game is now over
			if(m_iCurMode == PLAYING && !movePossible())
				changeMode(GAMEOVER);
			
			//Make photo-taking noise if we should
			if(m_cam->isOpen() && m_iCurMode == GAMEOVER)
			{
				if(!m_bSavedFacepic && getSeconds() > m_fGameoverWebcamFreeze + 0.5)
				{
					m_bSavedFacepic = true;
					playSound("camera", m_fSoundVolume);
				}
			}
			break;
		
		case INTRO:
		{			
			//Calculate the proper alpha value for the black cover-up-intro graphic for our fadein time
			float alpha = (getSeconds() - INTRO_FADEIN_DELAY) / INTRO_FADEIN_TIME;
			if(alpha < 0) alpha = 0;
			if(alpha > 1) alpha = 1;
			HUDItem* hIt = m_hud->getChild("coverintro");
			if(hIt != NULL)
				hIt->col.a = 1.0-alpha;
			break;
		}
		
		case SONGSELECT:
			beatDetect();	//Bounce some menu stuff to the beat
			m_fMusicScrubSpeed += dt * MUSIC_SCRUBIN_SPEED;
			if(m_fMusicScrubSpeed > soundFreqDefault)
				m_fMusicScrubSpeed = soundFreqDefault;
			setMusicFrequency(m_fMusicScrubSpeed);
			m_selectedSongArc->update(dt);
			for(vector<ParticleSystem*>::iterator i = m_selectedSongParticles.begin(); i != m_selectedSongParticles.end(); i++)
				(*i)->update(dt);
			for(list<ParticleSystem*>::iterator i = m_selectedSongParticlesBg.begin(); i != m_selectedSongParticlesBg.end(); i++)
				(*i)->update(dt);
			
			//Check and see if we should change bg colors
			if(m_bg != NULL && m_bg->type == GRADIENT)
			{
				gradientBg* bg = (gradientBg*)m_bg;
				bool ur, br, bl, ul;
				ur = br = bl = ul = false;
				for(list<ColorPhase>::iterator i = m_ColorsChanging.begin(); i != m_ColorsChanging.end(); i++)
				{
					if(i->colorToChange == &bg->ur)
						ur = true;
					if(i->colorToChange == &bg->br)
						br = true;
					if(i->colorToChange == &bg->ul)
						ul = true;
					if(i->colorToChange == &bg->bl)
						bl = true;
				}
				if(!ur)
					phaseColor(&bg->ur, Color(randFloat(0,1),randFloat(0,1),randFloat(0,1),0), randFloat(0.15,0.35));
				if(!br)
					phaseColor(&bg->br, Color(randFloat(0,1),randFloat(0,1),randFloat(0,1),0), randFloat(0.15,0.35));
				if(!ul)
					phaseColor(&bg->ul, Color(randFloat(0,1),randFloat(0,1),randFloat(0,1),0), randFloat(0.15,0.35));
				if(!bl)
					phaseColor(&bg->bl, Color(randFloat(0,1),randFloat(0,1),randFloat(0,1),0), randFloat(0.15,0.35));
			}
			break;
	}
	updateColors(dt);
}

void Pony48Engine::draw()
{
	//Clear bg (not done with OpenGL funcs, cause of weird black frame glitch when loading stuff)
	fillScreen(m_BgCol);
	glClear(GL_DEPTH_BUFFER_BIT);
	
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
			}
			
			glClear(GL_DEPTH_BUFFER_BIT);
			
			//Draw particle system
			for(map<string, ParticleSystem*>::iterator i = songParticles.begin(); i != songParticles.end(); i++)
				i->second->draw();
			
			//Draw webcam stuffz right in front of that
			if(m_cam->isOpen() && m_iCurMode == PLAYING)
			{
				glColor4f(1,1,1,1);
#ifndef USE_VIDEOINPUT
				if(++m_iCurCamFrameSkip >= m_iCAM_FRAME_SKIP)
				{
					m_iCurCamFrameSkip = 0;
#endif
					m_cam->getNewFrame();
#ifndef USE_VIDEOINPUT
				}
#endif
				if(m_bDrawFacecam)
					m_cam->draw(m_fWebcamDrawSize, m_ptWebcamDrawPos);
			}
			
			glClear(GL_DEPTH_BUFFER_BIT);
			
			//Set up OpenGL matrices
			glLoadIdentity();
			
			glRotatef(m_fSongFxRotate, 0.0f, 0.0f, 1.0f);	//Rotate according to song fx
			glTranslatef(CameraPos.x, CameraPos.y, CameraPos.z);	//Translate according to where our camera is
			glRotatef(m_BoardRotAngle, m_BoardRot.x, m_BoardRot.y, m_BoardRot.z);	//Rotate according to what direction the player is pressing
			
			
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
			
			float32 fSec = getSeconds();
			txt = (HUDTextbox*)m_hud->getChild("title");
			if(fSec > m_fFadeoutTitleTime)
			{
				txt->col.a = max((TITLE_FADE_TIME - (fSec - m_fFadeoutTitleTime)), 0.0f);
				txt = (HUDTextbox*)m_hud->getChild("artist");
				txt->col.a = max((TITLE_FADE_TIME - (fSec - m_fFadeoutTitleTime)), 0.0f);
			}
			//oss << 1.0 / (fSec - m_fLastFrame);
			//txt->setText(oss.str());
			//m_fLastFrame = fSec;
			break;
		}
		
		case SONGSELECT:
		{
			if(m_bg != NULL)
				m_bg->draw();
			glClear(GL_DEPTH_BUFFER_BIT);
			for(list<ParticleSystem*>::iterator i = m_selectedSongParticlesBg.begin(); i != m_selectedSongParticlesBg.end(); i++)
				(*i)->draw();
			HUDItem* hIt = m_hud->getChild("songmenu");
			if(hIt != NULL)
			{
				HUDMenu* hMen = (HUDMenu*)hIt;
				glTranslatef(0, hMen->selectedY, 0);
				m_selectedSongArc->p1.Set(-hMen->selectedX-3, m_selectedSongArc->height / 2.0f);
				m_selectedSongArc->p2.Set(hMen->selectedX+3, m_selectedSongArc->height / 2.0f);
			}
			m_selectedSongArc->draw();
			for(vector<ParticleSystem*>::iterator i = m_selectedSongParticles.begin(); i != m_selectedSongParticles.end(); i++)
				(*i)->draw();
			break;
		}
	}
	
	//Draw HUD always at this depth, on top of everything else
	glClear(GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0, 0, m_fDefCameraZ);
		
	//Draw HUD
	m_hud->draw(0);
	
	if(m_iCurMode == GAMEOVER)
	{
		//If webcam there, draw reaction image
		if(m_cam->isOpen())
		{
			glColor4f(1,1,1,1);
#ifdef USE_VIDEOINPUT
			if(getSeconds() < m_fGameoverWebcamFreeze)
#else
			if(++m_iCurCamFrameSkip >= m_iCAM_FRAME_SKIP && getSeconds() < m_fGameoverWebcamFreeze)
#endif
			{
#ifndef USE_VIDEOINPUT
				m_iCurCamFrameSkip = 0;
#endif
				m_cam->getNewFrame();
			}
			m_cam->draw(5, Point(0,-1.5));
		}
		else if(m_highestTile && m_highestTile->seg && m_highestTile->bg)	//Otherwise, draw higest tile the player got
		{
			glPushMatrix();
			glTranslatef(0, -1.5, 0);
			glRotatef(m_gameoverTileRot, 0, 0, 1);
			m_highestTile->seg->size = m_highestTile->bg->size = Point(4,4);
			float32 fHighestTileAlpha = m_highestTile->bg->col.a;
			m_highestTile->bg->col.a = 1.0f;	//Draw this at full opacity, even if in the actual game the color has changed
			m_highestTile->bg->draw();
			m_highestTile->bg->col.a = fHighestTileAlpha;
			m_highestTile->seg->draw();
			glPopMatrix();
		}
	}
	else if(m_iCurMode == PLAYING)
	{
		for(map<string, ParticleSystem*>::iterator i = m_ScoreParticles.begin(); i != m_ScoreParticles.end(); i++)
			i->second->draw();
	}
	
	//Set mouse cursor to proper location
	for(map<string, myCursor*>::iterator i = m_mCursors.begin(); i != m_mCursors.end(); i++)
	{
		i->second->pos = worldPosFromCursor(getCursorPos());
		if(i->first == "dir")
		{
			i->second->rot = -RAD2DEG * atan2(i->second->pos.x, i->second->pos.y) + 90;
			/*switch(getDirOfVec2(i->second->pos))
			{
				case UP:
					i->second->rot = 90;
					break;
					
				case DOWN:
					i->second->rot = -90;
					break;
					
				case LEFT:
					i->second->rot = 180;
					break;
					
				case RIGHT:
					i->second->rot = 1;
					break;
			}*/
		}
	}
	glColor4f(1,1,1,1);
}

void Pony48Engine::init(list<commandlineArg> sArgs)
{
	//Run through list for arguments we recognize
	for(list<commandlineArg>::iterator i = sArgs.begin(); i != sArgs.end(); i++)
	{
		errlog << "Commandline argument. Switch: " << i->sSwitch << ", value: " << i->sValue << endl;
	}
	
	//Load our last screen position and such
	if(!loadConfig(getSaveLocation() + "config.xml"))
		m_cam->open(m_iCAM);	//Open webcam if config loading fails
	
	//Set gravity to 0
	getWorld()->SetGravity(b2Vec2(0,0));
	
	//Init board
	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
			m_Board[j][i] = NULL;
	}
	resetBoard();
	
	m_imgMouseMoveArrow = getImage("res/movearrow.png");
	
	//Create sounds up front
	createSound("res/sfx/jointile.ogg", "jointile");
	createSound("res/sfx/select.ogg", "select");
	createSound("res/sfx/camera_shutter.ogg", "camera");
	createSound("res/vox/nowhacking_theyreponies.ogg", "nowhacking_theyreponies");
	
	ParticleSystem* pSys = new ParticleSystem();
	pSys->fromXML("res/particles/selectsong0.xml");
	pSys->init();
	pSys->firing = true;
	m_selectedSongParticles.push_back(pSys);
	m_selectedSongParticlesRateMul.push_back(1500);
	m_selectedSongParticlesThresh.push_back(0.25);
	pSys = new ParticleSystem();
	pSys->fromXML("res/particles/selectsong1.xml");
	pSys->init();
	pSys->firing = true;
	m_selectedSongParticles.push_back(pSys);
	m_selectedSongParticlesRateMul.push_back(3000);
	m_selectedSongParticlesThresh.push_back(0.1);
	pSys = new ParticleSystem();
	pSys->fromXML("res/particles/selectsong2.xml");
	pSys->init();
	pSys->firing = true;
	m_selectedSongParticles.push_back(pSys);
	m_selectedSongParticlesRateMul.push_back(6000);
	m_selectedSongParticlesThresh.push_back(0.1);
	pSys = new ParticleSystem();
	pSys->fromXML("res/particles/selectsong3.xml");
	pSys->init();
	pSys->firing = true;
	m_selectedSongParticles.push_back(pSys);
	m_selectedSongParticlesRateMul.push_back(8000);
	m_selectedSongParticlesThresh.push_back(0.1);
	pSys = new ParticleSystem();
	pSys->fromXML("res/particles/selectsong4.xml");
	pSys->init();
	pSys->firing = true;
	m_selectedSongParticles.push_back(pSys);
	m_selectedSongParticlesRateMul.push_back(9001);	//IT'S OVER NINE THOU- *shot*
	m_selectedSongParticlesThresh.push_back(0.1);
	
	//Add bg spinning 2048 tile particle system
	pSys = new ParticleSystem();
	pSys->fromXML("res/particles/bg2048ul.xml");
	pSys->init();
	pSys->firing = true;
	m_selectedSongParticlesBg.push_back(pSys);
	pSys = new ParticleSystem();
	pSys->fromXML("res/particles/bg2048ur.xml");
	pSys->init();
	pSys->firing = true;
	m_selectedSongParticlesBg.push_back(pSys);
	pSys = new ParticleSystem();
	pSys->fromXML("res/particles/bg2048bl.xml");
	pSys->init();
	pSys->firing = true;
	m_selectedSongParticlesBg.push_back(pSys);
	pSys = new ParticleSystem();
	pSys->fromXML("res/particles/bg2048br.xml");
	pSys->init();
	pSys->firing = true;
	m_selectedSongParticlesBg.push_back(pSys);
	//HACK Make this look ok on the first frame by fake-updating it for a bit
	//for(int i = 0; i < 60; i++)
	//	pSys->update(0.25);
	
	INTRO_FADEIN_DELAY = 1.0 + getSeconds();
}


void Pony48Engine::hudSignalHandler(string sSignal)
{
	if(m_iCurMode == SONGSELECT)
	{
		if(sSignal.find("load ") == 0)
		{
			m_sSongToPlay = sSignal.substr(5);
			changeMode(PLAYING);
		}
		else if(sSignal == "select")
		{
			playSound("select", m_fSoundVolume);
		}
	}
}

void Pony48Engine::handleEvent(SDL_Event event)
{
	if(m_hud->event(event)) return;	//Let our HUD handle any events it needs to, and back out if it got handled
	switch(event.type)
	{
		//Key pressed
		case SDL_KEYDOWN:
		{
			if(event.key.keysym.scancode != SDL_SCANCODE_ESCAPE)
			{
				m_bJoyControl = false;
				m_iMouseControl = 0;
				hideCursor();
			}
			if(m_iCurMode == GAMEOVER)
			{
				if(!(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE
#ifdef DEBUG
					 || event.key.keysym.scancode == SDL_SCANCODE_F5
#endif
					 ) && getSeconds() - m_fGameoverKeyDelay >= GAMEOVER_KEY_DELAY)	//Wait for a certain amount of time, in case they just mashed keys accidentally
				{
					changeMode(PLAYING);
					break;
				}
				else if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
					changeMode(SONGSELECT);
#ifdef DEBUG
				else if(event.key.keysym.scancode == SDL_SCANCODE_F5)
				{
					string sScene = m_hud->getScene();
					clearColors();
					delete m_hud;
					m_hud = new HUD("hud");
					m_hud->create("res/hud.xml");
					m_hud->setScene(sScene);
					m_hud->setSignalHandler(signalHandler);
					//Reload particles
					for(map<string, ParticleSystem*>::iterator i = songParticles.begin(); i != songParticles.end(); i++)
					{
						bool f = i->second->firing;
						bool s = i->second->show;
						i->second->reload();
						i->second->firing = f;
						i->second->show = s;
						i->second->init();
					}
					break;
				}
#endif
			}
			else if(m_iCurMode == PLAYING)
			{
				switch(event.key.keysym.scancode)
				{
					case SDL_SCANCODE_ESCAPE:
						changeMode(SONGSELECT);
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
						clearColors();
						delete m_hud;
						m_hud = new HUD("hud");
						m_hud->create("res/hud.xml");
						m_hud->setScene(sScene);
						m_hud->setSignalHandler(signalHandler);
						//Reload particles
						for(map<string, ParticleSystem*>::iterator i = songParticles.begin(); i != songParticles.end(); i++)
						{
							bool f = i->second->firing;
							bool s = i->second->show;
							i->second->reload();
							i->second->firing = f;
							i->second->show = s;
							i->second->init();
						}
						break;
					}
#endif
					case SDL_SCANCODE_RETURN:
						if(keyDown(SDL_SCANCODE_ALT))
							toggleFullscreen();
						break;
						
					default:
						//Deal with movement
						if(event.key.keysym.scancode == KEY_UP1 || event.key.keysym.scancode == KEY_UP2)
						{
							if(m_iCurMode == PLAYING)
								move(UP);
						}
						else if(event.key.keysym.scancode == KEY_DOWN1 || event.key.keysym.scancode == KEY_DOWN2)
						{
							if(m_iCurMode == PLAYING)
								move(DOWN);
						}
						else if(event.key.keysym.scancode == KEY_LEFT1 || event.key.keysym.scancode == KEY_LEFT2)
						{
							if(m_iCurMode == PLAYING)
								move(LEFT);
						}
						else if(event.key.keysym.scancode == KEY_RIGHT1 || event.key.keysym.scancode == KEY_RIGHT2)
						{
							if(m_iCurMode == PLAYING)
								move(RIGHT);
						}
						break;
				}
				break;
			}
			else if(m_iCurMode == INTRO)
			{
#ifdef DEBUG
				if(event.key.keysym.scancode == SDL_SCANCODE_F5)
				{
					string sScene = m_hud->getScene();
					clearColors();
					delete m_hud;
					m_hud = new HUD("hud");
					m_hud->create("res/hud.xml");
					m_hud->setScene(sScene);
					m_hud->setSignalHandler(signalHandler);
				}
				else
#endif
					changeMode(SONGSELECT);
			}
			else if(m_iCurMode == SONGSELECT)
			{
				if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
					quit();
			}
		}
		
		//Key released
		case SDL_KEYUP:
			switch(event.key.keysym.scancode)
			{
			}
			break;
		
		case SDL_MOUSEBUTTONDOWN:
#ifdef DEBUG_INPUT
			cout << "Mouse button " << (int)event.button.button << " pressed." << endl;
#endif
			if(m_iMouseControl >= MOUSE_MOVE_TRIP_AMT && m_iCurMode == PLAYING)
				move(getDirOfVec2(worldPosFromCursor(getCursorPos())));	//Nested functions much?
			if(m_iCurMode == GAMEOVER)
				changeMode(PLAYING);
			else if(m_iCurMode == INTRO)
				changeMode(SONGSELECT);
			m_iMouseControl = MOUSE_MOVE_TRIP_AMT;
			showCursor();
			m_bJoyControl = false;
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
#ifdef DEBUG_INPUT
			cout << "Mouse button " << (int)event.button.button << " released." << endl;
#endif
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
			if(++m_iMouseControl >= MOUSE_MOVE_TRIP_AMT)
			{
				showCursor();
				m_bJoyControl = false;
			}
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
					
				case SDL_WINDOWEVENT_LEAVE:
					m_iMouseControl = MOUSE_MOVE_TRIP_AMT - 1;	//Make board tilt back if the mouse exits the window
					break;
			}
			break;
			
		//Gamepad stuff!
		case SDL_JOYDEVICEADDED:
			errlog << "Joystick " << (int)event.jdevice.which << " connected." << endl;
			m_joy = SDL_JoystickOpen(event.jdevice.which);

			if(m_joy) 
			{
				m_bJoyControl = true;
				errlog << "Opened Joystick " << event.jdevice.which << endl;
				errlog << "Name: " << SDL_JoystickNameForIndex(event.jdevice.which) << endl;
				errlog << "Number of Axes: " << SDL_JoystickNumAxes(m_joy) << endl;
				errlog << "Number of Buttons: " << SDL_JoystickNumButtons(m_joy) << endl;
				errlog << "Number of Balls: " << SDL_JoystickNumBalls(m_joy) << endl;
				errlog << "Number of Hats: " << SDL_JoystickNumHats(m_joy) << endl;
				
				//On Linux, "xboxdrv" is the driver I had the most success with when it came to rumble (default driver said it rumbled, but didn't)
				m_rumble = NULL;
				if(SDL_JoystickIsHaptic(m_joy))
					m_rumble = SDL_HapticOpenFromJoystick(m_joy);
				if(m_rumble)
				{
					if(SDL_HapticRumbleInit(m_rumble) != 0)
					{
						errlog << "Error initializing joystick " << (int)event.jdevice.which << " as rumble." << endl;
						SDL_HapticClose(m_rumble);
						m_rumble = NULL;
					}
					else 
					{
						errlog << "Initialized joystick " << (int)event.jdevice.which << " as rumble." << endl;
					}
				}
				else
					errlog << "Joystick " << (int)event.jdevice.which << " has no rumble support." << endl;
			} 
			else
				errlog << "Couldn't open Joystick " << (int)event.jdevice.which << endl;
			break;
			
		case SDL_JOYDEVICEREMOVED:
			errlog << "Joystick " << (int)event.jdevice.which << " disconnected." << endl;
			break;
			
		case SDL_JOYBUTTONDOWN:
#ifdef DEBUG_INPUT
			cout << "Joystick " << (int)event.jbutton.which << " pressed button " << (int)event.jbutton.button << endl;
#endif
			m_iMouseControl = 0;
			hideCursor();
			m_bJoyControl = true;
			if(event.jbutton.button == JOY_BUTTON_START)
			{
				if(m_iCurMode == PLAYING || m_iCurMode == GAMEOVER)
					changeMode(SONGSELECT);
				else
					quit();
			}
			else if(m_iCurMode == GAMEOVER)
				changeMode(PLAYING);
			else if(m_iCurMode == INTRO)
				changeMode(SONGSELECT);
			break;
			
		case SDL_JOYBUTTONUP:
#ifdef DEBUG_INPUT
			cout << "Joystick " << (int)event.jbutton.which << " released button " << (int)event.jbutton.button << endl;
#endif
			break;
			
		case SDL_JOYAXISMOTION:
			if(abs(event.jaxis.value) > JOY_MINMOVE_TRIP)
			{
				m_iMouseControl = 0;
				hideCursor();
				m_bJoyControl = true;
#ifdef DEBUG_INPUT
				cout << "Joystick " << (int)event.jaxis.which << " moved axis " << (int)event.jaxis.axis << " to " << event.jaxis.value << endl;
#endif
			}
			
			if(event.jaxis.axis == JOY_AXIS_HORIZ)	//Horizontal axis
			{
				if(event.jaxis.value < -JOY_AXIS_TRIP)	//Left
				{
					if(m_iCurMode == PLAYING && !bJoyHorizontalMove)
						move(LEFT);
					bJoyHorizontalMove = true;
				}
				else if(event.jaxis.value > JOY_AXIS_TRIP)	//Right
				{
					if(m_iCurMode == PLAYING && !bJoyHorizontalMove)
						move(RIGHT);
					bJoyHorizontalMove = true;
				}
				else
					bJoyHorizontalMove = false;
			}
			else if(event.jaxis.axis == JOY_AXIS_VERT)	//Vertical axis
			{
				if(event.jaxis.value < -JOY_AXIS_TRIP)	//Up
				{
					if(m_iCurMode == PLAYING && !bJoyVerticalMove)
						move(UP);
					bJoyVerticalMove = true;
				}
				else if(event.jaxis.value > JOY_AXIS_TRIP)	//Down
				{
					if(m_iCurMode == PLAYING && !bJoyVerticalMove)
						move(DOWN);
					bJoyVerticalMove = true;
				}
				else
					bJoyVerticalMove = false;
			}
			//Second stick pans view around because why not
			else if(event.jaxis.axis == JOY_AXIS2_HORIZ)
			{
				CameraPos.x = (float32)event.jaxis.value / (float32)JOY_AXIS_MAX * 2.0;
			}
			else if(event.jaxis.axis == JOY_AXIS2_VERT)
			{
				CameraPos.y = (float32)event.jaxis.value / (float32)JOY_AXIS_MIN * 2.0;
			}
			else if(event.jaxis.axis == JOY_AXIS_RT)
			{
				if(event.jaxis.value > 0)	//Pressed more than halfway; behave like button
				{
					if(m_iCurMode == GAMEOVER)
						changeMode(PLAYING);
					else if(m_iCurMode == INTRO)
						changeMode(SONGSELECT);
				}
#ifdef DEBUG	//DEBUG: right trigger fast-forwards music
				FMOD_CHANNEL* channel = getChannel("music");
				float curval = event.jaxis.value;
				curval -= JOY_AXIS_MIN;	//Get absolute value of axis, from 0 to 65,535
				curval /= (float)(JOY_AXIS_MAX-JOY_AXIS_MIN);
				curval *= 12;	//Max speed
				FMOD_Channel_SetFrequency(channel, soundFreqDefault+soundFreqDefault*curval);
#endif
			}
			else if(event.jaxis.axis == JOY_AXIS_LT)
			{
				if(event.jaxis.value > 0)	//Pressed more than halfway; behave like button
				{
					if(m_iCurMode == GAMEOVER)
						changeMode(PLAYING);
					else if(m_iCurMode == INTRO)
						changeMode(SONGSELECT);
				}
#ifdef DEBUG_REVSOUND	//DEBUG: left trigger rewinds music
				FMOD_CHANNEL* channel = getChannel("music");
				float curval = event.jaxis.value;
				curval -= JOY_AXIS_MIN;	//Get absolute value of axis, from 0 to 65,535
				curval /= (float)(JOY_AXIS_MAX-JOY_AXIS_MIN);
				curval *= -5;
				FMOD_Channel_SetFrequency(channel, soundFreqDefault+soundFreqDefault*curval);
#endif
			}
			break;
			
		case SDL_JOYHATMOTION:
#ifdef DEBUG_INPUT
			cout << "Joystick " << (int)event.jhat.which << " moved hat " << (int)event.jhat.hat << " to " << (int)event.jhat.value << endl;
#endif
			m_lastJoyHatMoved = event.jhat.which;
			if(m_iCurMode == GAMEOVER && event.jhat.value && getSeconds() - m_fGameoverKeyDelay >= GAMEOVER_KEY_DELAY)
				changeMode(PLAYING);
			else if(m_iCurMode == INTRO && event.jhat.value)
				changeMode(SONGSELECT);
			else if(m_iCurMode == PLAYING)
			{
				switch(event.jhat.value)
				{
					case SDL_HAT_UP:
						move(UP);
						break;
						
					case SDL_HAT_DOWN:
						move(DOWN);
						break;
						
					case SDL_HAT_LEFT:
						move(LEFT);
						break;
						
					case SDL_HAT_RIGHT:
						move(RIGHT);
						break;
				}
				
				if(event.jhat.value)
				{
					m_iMouseControl = 0;
					hideCursor();
					m_bJoyControl = true;
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
	m_fLastMovedSec = getSeconds();	//don't play bored vox if we've minimized or been in bg
}

Rect Pony48Engine::getCameraView()
{
	Rect rcCamera;
	const float32 tan45_2 = tan(DEG2RAD*45/2);
	const float32 fAspect = (float32)getWidth() / (float32)getHeight();
	rcCamera.bottom = (tan45_2 * m_fDefCameraZ);
	rcCamera.top = -(tan45_2 * m_fDefCameraZ);
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

bool Pony48Engine::loadConfig(string sFilename)
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
		return false;	//No file; ignore
	}
	
	//Grab root element
	XMLElement* root = doc->RootElement();
	if(root == NULL)
	{
		errlog << "Error: Root element NULL in XML file. Ignoring..." << endl;
		if(isFullscreen())
			setInitialFullscreen();
		delete doc;
		return false;
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
		bool bTexAntialias = getImgBlur();
		float32 fGamma = getGamma();
		
		window->QueryUnsignedAttribute("width", &width);
		window->QueryUnsignedAttribute("height", &height);
		window->QueryBoolAttribute("fullscreen", &bFullscreen);
		window->QueryBoolAttribute("maximized", &bMaximized);
		window->QueryUnsignedAttribute("fps", &framerate);
		window->QueryBoolAttribute("doublebuf", &bDoubleBuf);
		window->QueryIntAttribute("vsync", &iVsync);
		window->QueryIntAttribute("MSAA", &iMSAA);
		window->QueryBoolAttribute("textureantialias", &bTexAntialias);
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
		setImgBlur(bTexAntialias);
		setGamma(fGamma);
		pauseOnKeyboard(bPausesOnFocus);
	}
	
	XMLElement* pony48 = root->FirstChildElement("pony48");
	if(pony48 != NULL)
	{
		pony48->QueryUnsignedAttribute("highscore", &m_iHighScore);
		pony48->QueryFloatAttribute("musicvol", &m_fMusicVolume);
		pony48->QueryFloatAttribute("soundvol", &m_fSoundVolume);
		pony48->QueryFloatAttribute("voxvol", &m_fVoxVolume);
	}
	
	XMLElement* joystick = root->FirstChildElement("joystick");
	if(joystick != NULL)
	{
		joystick->QueryIntAttribute("axistripthreshold", &JOY_AXIS_TRIP);
		joystick->QueryUnsignedAttribute("backbutton", &JOY_BUTTON_BACK);
		joystick->QueryUnsignedAttribute("startbutton", &JOY_BUTTON_START);
		joystick->QueryUnsignedAttribute("Y", &JOY_BUTTON_Y);
		joystick->QueryUnsignedAttribute("X", &JOY_BUTTON_X);
		joystick->QueryUnsignedAttribute("A", &JOY_BUTTON_A);
		joystick->QueryUnsignedAttribute("B", &JOY_BUTTON_B);
		joystick->QueryUnsignedAttribute("LB", &JOY_BUTTON_LB);
		joystick->QueryUnsignedAttribute("RB", &JOY_BUTTON_RB);
		joystick->QueryUnsignedAttribute("leftstick", &JOY_BUTTON_LSTICK);
		joystick->QueryUnsignedAttribute("rightstick", &JOY_BUTTON_RSTICK);
		joystick->QueryUnsignedAttribute("horizontalaxis1", &JOY_AXIS_HORIZ);
		joystick->QueryUnsignedAttribute("verticalaxis1", &JOY_AXIS_VERT);
		joystick->QueryUnsignedAttribute("horizontalaxis2", &JOY_AXIS2_HORIZ);
		joystick->QueryUnsignedAttribute("verticalaxis2", &JOY_AXIS2_VERT);
		joystick->QueryUnsignedAttribute("ltaxis", &JOY_AXIS_LT);
		joystick->QueryUnsignedAttribute("rtaxis", &JOY_AXIS_RT);
	}
	
	XMLElement* keyboard = root->FirstChildElement("keyboard");
	if(keyboard != NULL)
	{
		const char* cUpKey1 = keyboard->Attribute("upkey1");
		const char* cUpKey2 = keyboard->Attribute("upkey2");
		const char* cDownKey1 = keyboard->Attribute("downkey1");
		const char* cDownKey2 = keyboard->Attribute("downkey2");
		const char* cLeftKey1 = keyboard->Attribute("leftkey1");
		const char* cLeftKey2 = keyboard->Attribute("leftkey2");
		const char* cRightKey1 = keyboard->Attribute("rightkey1");
		const char* cRightKey2 = keyboard->Attribute("rightkey2");
		const char* cEnter1 = keyboard->Attribute("enter1");
		const char* cEnter2 = keyboard->Attribute("enter2");
		if(cUpKey1)
			KEY_UP1 = SDL_GetScancodeFromName(cUpKey1);
		if(cUpKey2)
			KEY_UP2 = SDL_GetScancodeFromName(cUpKey2);
		if(cDownKey1)
			KEY_DOWN1 = SDL_GetScancodeFromName(cDownKey1);
		if(cDownKey2)
			KEY_DOWN2 = SDL_GetScancodeFromName(cDownKey2);
		if(cLeftKey1)
			KEY_LEFT1 = SDL_GetScancodeFromName(cLeftKey1);
		if(cLeftKey2)
			KEY_LEFT2 = SDL_GetScancodeFromName(cLeftKey2);
		if(cRightKey1)
			KEY_RIGHT1 = SDL_GetScancodeFromName(cRightKey1);
		if(cRightKey2)
			KEY_RIGHT2 = SDL_GetScancodeFromName(cRightKey2);
		if(cEnter1)
			KEY_ENTER1 = SDL_GetScancodeFromName(cEnter1);
		if(cEnter2)
			KEY_ENTER2 = SDL_GetScancodeFromName(cEnter2);
	}
	
	XMLElement* webcam = root->FirstChildElement("cam");
	if(webcam != NULL)
	{
		webcam->QueryIntAttribute("device", &m_iCAM);
#ifndef USE_VIDEOINPUT
		webcam->QueryIntAttribute("frameskip", &m_iCAM_FRAME_SKIP);
#endif
		const char* facepos = webcam->Attribute("facepos");
		if(facepos != NULL)
			m_ptWebcamDrawPos = pointFromString(facepos);
		webcam->QueryFloatAttribute("facesz", &m_fWebcamDrawSize);
		webcam->QueryBoolAttribute("drawfacecam", &m_bDrawFacecam);
		webcam->QueryBoolAttribute("use", &m_cam->use);
	}
	m_cam->open(m_iCAM);
	
	delete doc;
	return true;
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
	window->SetAttribute("textureantialias", getImgBlur());
	window->SetAttribute("brightness", getGamma());
	window->SetAttribute("pauseminimized", pausesOnFocusLost());
	root->InsertEndChild(window);
	
	XMLElement* pony48 = doc->NewElement("pony48");
	pony48->SetAttribute("highscore", m_iHighScore);
	pony48->SetAttribute("musicvol", m_fMusicVolume);
	pony48->SetAttribute("soundvol", m_fSoundVolume);
	pony48->SetAttribute("voxvol", m_fVoxVolume);
	root->InsertEndChild(pony48);
	
	XMLElement* joystick = doc->NewElement("joystick");
	joystick->SetAttribute("axistripthreshold", JOY_AXIS_TRIP);
	joystick->SetAttribute("backbutton", JOY_BUTTON_BACK);
	joystick->SetAttribute("startbutton", JOY_BUTTON_START);
	joystick->SetAttribute("Y", JOY_BUTTON_Y);
	joystick->SetAttribute("X", JOY_BUTTON_X);
	joystick->SetAttribute("A", JOY_BUTTON_A);
	joystick->SetAttribute("B", JOY_BUTTON_B);
	joystick->SetAttribute("LB", JOY_BUTTON_LB);
	joystick->SetAttribute("RB", JOY_BUTTON_RB);
	joystick->SetAttribute("leftstick", JOY_BUTTON_LSTICK);
	joystick->SetAttribute("rightstick", JOY_BUTTON_RSTICK);
	joystick->SetAttribute("horizontalaxis1", JOY_AXIS_HORIZ);
	joystick->SetAttribute("verticalaxis1", JOY_AXIS_VERT);
	joystick->SetAttribute("horizontalaxis2", JOY_AXIS2_HORIZ);
	joystick->SetAttribute("verticalaxis2", JOY_AXIS2_VERT);
	joystick->SetAttribute("ltaxis", JOY_AXIS_LT);
	joystick->SetAttribute("rtaxis", JOY_AXIS_RT);
	root->InsertEndChild(joystick);
	
	XMLElement* keyboard = doc->NewElement("keyboard");
	keyboard->SetAttribute("upkey1", SDL_GetScancodeName(KEY_UP1));
	keyboard->SetAttribute("upkey2", SDL_GetScancodeName(KEY_UP2));
	keyboard->SetAttribute("downkey1", SDL_GetScancodeName(KEY_DOWN1));
	keyboard->SetAttribute("downkey2", SDL_GetScancodeName(KEY_DOWN2));
	keyboard->SetAttribute("leftkey1", SDL_GetScancodeName(KEY_LEFT1));
	keyboard->SetAttribute("leftkey2", SDL_GetScancodeName(KEY_LEFT2));
	keyboard->SetAttribute("rightkey1", SDL_GetScancodeName(KEY_RIGHT1));
	keyboard->SetAttribute("rightkey2", SDL_GetScancodeName(KEY_RIGHT2));
	keyboard->SetAttribute("enter1", SDL_GetScancodeName(KEY_ENTER1));
	keyboard->SetAttribute("enter2", SDL_GetScancodeName(KEY_ENTER2));
	root->InsertEndChild(keyboard);
	
	XMLElement* webcam = doc->NewElement("cam");
	webcam->SetAttribute("device", m_iCAM);
#ifndef USE_VIDEOINPUT
	webcam->SetAttribute("frameskip", m_iCAM_FRAME_SKIP);
#endif
	webcam->SetAttribute("facepos", pointToString(m_ptWebcamDrawPos).c_str());
	webcam->SetAttribute("facesz", m_fWebcamDrawSize);
	webcam->SetAttribute("drawfacecam", m_bDrawFacecam);
	webcam->SetAttribute("use", m_cam->use);
	root->InsertEndChild(webcam);
	
	doc->InsertFirstChild(root);
	doc->SaveFile(sFilename.c_str());
	delete doc;
}

obj* Pony48Engine::objFromXML(string sXMLFilename, Point ptOffset, Point ptVel)
{
	return NULL;
}

#define BOARD_ROT_AMT		6.0
#define MAX_BOARD_ROT_ANGLE	30.0
#define SQRT_2				1.41421356237

void Pony48Engine::handleKeys()
{
#ifdef DEBUG
	if(keyDown(SDL_SCANCODE_B))
		setTimeScale(DEFAULT_TIMESCALE/3);
	else
		setTimeScale(DEFAULT_TIMESCALE);
#endif
	//Check joystick movement
	Sint16 x_move = 0;
	Sint16 y_move = 0;
	if(m_joy && SDL_JoystickGetAttached(m_joy))
	{
		x_move = SDL_JoystickGetAxis(m_joy, JOY_AXIS_HORIZ);
		y_move = SDL_JoystickGetAxis(m_joy, JOY_AXIS_VERT);
		if(abs(x_move) < JOY_MINMOVE_TRIP && abs(y_move) < JOY_MINMOVE_TRIP)
			x_move = y_move = 0;
	}
	if(m_iMouseControl >= MOUSE_MOVE_TRIP_AMT && m_iCurMode == PLAYING)
	{
		Point ptMouse = worldPosFromCursor(getCursorPos());
		ptMouse *= (float32)JOY_AXIS_MAX / (getCameraView().height()*1.5);
		x_move = min(max(-ptMouse.x, (float32)JOY_AXIS_MIN), (float32)JOY_AXIS_MAX);
		y_move = min(max(ptMouse.y, (float32)JOY_AXIS_MIN), (float32)JOY_AXIS_MAX);
	}
	Point vecMove((float32)x_move/(float32)JOY_AXIS_MAX, (float32)-y_move/(float32)JOY_AXIS_MAX);
	
	//Check joystick hat movement
	if(m_joy && SDL_JoystickGetAttached(m_joy) && SDL_JoystickNumHats(m_joy) > m_lastJoyHatMoved)
	{
		switch(SDL_JoystickGetHat(m_joy, m_lastJoyHatMoved))
		{
			case SDL_HAT_UP:
				vecMove.y += 1.0f;
				break;

			case SDL_HAT_RIGHT:
				vecMove.x += 1.0;
				break;

			case SDL_HAT_DOWN:
				vecMove.y -= 1.0f;
				break;

			case SDL_HAT_LEFT:
				vecMove.x -= 1.0;
				break;

			case SDL_HAT_RIGHTUP:
				vecMove.y += 1.0f;
				vecMove.x += 1.0;
				break;

			case SDL_HAT_RIGHTDOWN:
				vecMove.y -= 1.0f;
				vecMove.x += 1.0;
				break;

			case SDL_HAT_LEFTUP:
				vecMove.y += 1.0f;
				vecMove.x -= 1.0;
				break;

			case SDL_HAT_LEFTDOWN:
				vecMove.y -= 1.0f;
				vecMove.x -= 1.0;
				break;
		}
	}
	
	//Check keyboard movement
	if(keyDown(SDL_SCANCODE_W) || keyDown(SDL_SCANCODE_UP))
		vecMove.y += 1.0;
	if(keyDown(SDL_SCANCODE_S) || keyDown(SDL_SCANCODE_DOWN))
		vecMove.y -= 1.0;
	if(keyDown(SDL_SCANCODE_A) || keyDown(SDL_SCANCODE_LEFT))
		vecMove.x -= 1.0;
	if(keyDown(SDL_SCANCODE_D) || keyDown(SDL_SCANCODE_RIGHT))
		vecMove.x += 1.0;
	
	//Cut length off to a maximum of 1.0
	float32 normalizeFac = vecMove.Length();
	if(normalizeFac > 1.0)
	{
		vecMove *= 1.0/normalizeFac;
		normalizeFac = 1.0;
	}
	
	float32 destAngle = normalizeFac * MAX_BOARD_ROT_ANGLE;
	//Rotate board towards destination rotate angle
	if(m_BoardRotAngle < destAngle)
	{
		m_BoardRotAngle += BOARD_ROT_AMT;
		if(m_BoardRotAngle > destAngle)
			m_BoardRotAngle = destAngle;
	}
	else if(m_BoardRotAngle > destAngle)
	{
		m_BoardRotAngle -= BOARD_ROT_AMT;
		if(m_BoardRotAngle < destAngle)
			m_BoardRotAngle = destAngle;
	}
	
	vecMove = rotateAroundPoint(vecMove, 90.0);	//Our rotate angle is the angle perpendicular to the direction we're pressing
	if(vecMove.x || vecMove.y)	//If this got reset to 0, let it gracefully rotate back around the last pressed vector
	{
		m_BoardRot.x = vecMove.x;
		m_BoardRot.y = vecMove.y;
	}
	
}

void Pony48Engine::changeMode(gameMode gm)
{
	clearColors();
	switch(gm)
	{
		case PLAYING:
			setCursor(m_mCursors["dir"]);
			if(m_iCurMode == GAMEOVER)
			{
				resetBoard();
				scrubResume();
			}
			else if(m_iCurMode == SONGSELECT)	//Start playing a song
			{
				m_fMusicPos["songselect"] = getMusicPos();
				resetBoard();
				loadSongXML(m_sSongToPlay);
				//if(m_fMusicPos.count(m_sSongToPlay))
				//	seekMusic(m_fMusicPos[m_sSongToPlay]);
			}
			m_hud->setScene("playing");
			break;
			
		case GAMEOVER:
		{
			setCursor(m_mCursors["sel"]);
			m_gameoverTileRot = 0;
			m_gameoverTileVel = 30;
			m_gameoverTileAccel = 16;
			//Play gameover rumble if ded
			rumbleController(1.0, 0.8, true);
			//Update final score counter
			ostringstream oss;
			HUDTextbox* txt = (HUDTextbox*)m_hud->getChild("finalscore");
			oss << "FINAL SCORE: " << m_iScore;
			if(txt != NULL)
				txt->setText(oss.str());
			m_hud->setScene("gameover");
			scrubPause();
			m_fGameoverKeyDelay = getSeconds();
			m_fGameoverWebcamFreeze = getSeconds() + GAMEOVER_FREEZE_CAM_TIME;
			m_bSavedFacepic = false;
			break;
		}
		
		case SONGSELECT:
		{
			gradientBg* bg = new gradientBg();
			bg->ul = Color(1,0,0,0.35);
			bg->ur = Color(0,1,0,0.35);
			bg->bl = Color(0,0,1,0.35);
			bg->br = Color(1,1,1,0.35);
			if(m_bg != NULL) 
				delete m_bg;
			m_bg = (Background*) bg;
			setCursor(m_mCursors["sel"]);
			m_fMusicPos[m_sSongToPlay] = getMusicPos();
			playMusic("res/mus/SleeplessNight.mp3", m_fMusicVolume);
			if(m_iCurMode == INTRO)
				m_fMusicScrubSpeed = soundFreqDefault;
			else
				m_fMusicScrubSpeed = 0;
			if(m_fMusicPos.count("songselect"))
				seekMusic(m_fMusicPos["songselect"]);
			else
				seekMusic(28.622f);
			//musicLoop(76.389f, 120.025f);
			m_hud->setScene("songselect");
			break;
		}
	}
	m_iCurMode = gm;
}

void Pony48Engine::rumbleController(float32 strength, float32 sec, bool priority)
{
	static float32 fLastRumble = 0.0f;
	if(priority)
		fLastRumble = getSeconds() + sec;
	else if(getSeconds() < fLastRumble)
		return;
	strength = max(strength, 0.0f);
	strength = min(strength, 1.0f);
	if(m_rumble != NULL && m_bJoyControl)
		SDL_HapticRumblePlay(m_rumble, strength, sec*1000);
}






























