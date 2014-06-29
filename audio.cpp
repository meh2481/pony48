/*
    Pony48 source - audio.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "Pony48.h"

const int sampleSize = 64;
static float32 startMenuPt = 0.0f;

void Pony48Engine::beatDetect()
{
	if(m_iCurMode == GAMEOVER) return;	//music stops when game is over, so it looks silly
	FMOD_CHANNEL* channel = getChannel("music");
	if(channel == NULL) return;
	
	
	//Code based off of http://katyscode.wordpress.com/2013/01/16/cutting-your-teeth-on-fmod-part-4-frequency-analysis-graphic-equalizer-beat-detection-and-bpm-estimation/
	float *specLeft, *specRight, *spec;

	specLeft = new float[sampleSize];
	specRight = new float[sampleSize];
	spec = new float[sampleSize];

	// Get spectrum for left and right stereo channels
	FMOD_Channel_GetSpectrum(channel, specLeft, sampleSize, 0, FMOD_DSP_FFT_WINDOW_RECT);
	FMOD_Channel_GetSpectrum(channel, specRight, sampleSize, 1, FMOD_DSP_FFT_WINDOW_RECT);

	//Center for a mono sound
	for(int i = 0; i < sampleSize; i++)
		spec[i] = (specLeft[i] + specRight[i]) / 2.0;
	
#ifdef DEBUG
	/*const int printLen = 150;
	//Print out a sort of audio-level thing
	printf("\033[2J\033[1;1H");
	for(int i = 0; i < 32; i++)
	{
		int num = spec[i] * printLen;
		for(int j = 0; j < num; j++)
			printf("*");
		for(int rest = num; rest < printLen; rest++)
			printf(" ");
		printf("|\n");
	}*/
#endif

	if(m_iCurMode == SONGSELECT)
	{
		beatThresholdVolume = 0.95;
		beatThresholdBar = 0;
		beatMul = 0.05f;
		maxCamz = 3.5f;
		m_fCamBounceBack = 0.02f;
		
		//Bounce parta da menu to da beat
		HUDItem* hIt = m_hud->getChild("choosesong");
		if(hIt != NULL)
		{
			HUDTextbox* hMen = (HUDTextbox*)hIt;
			if(!startMenuPt)
				startMenuPt = hMen->pt;
			
			//Bounce back
			if(hMen->pt > startMenuPt)
				hMen->pt -= m_fCamBounceBack;
			if(hMen->pt < startMenuPt)
				hMen->pt = startMenuPt;
			
			//Bounce forward
			if(spec[beatThresholdBar] >= beatThresholdVolume)
			{
				hMen->pt += beatMul * spec[beatThresholdBar];
				if(hMen->pt > startMenuPt + maxCamz)
					hMen->pt = startMenuPt + maxCamz;
			}
		}
		
		//Also make electric arc bounce to da beat
		m_selectedSongArc->max = 0.5 * spec[beatThresholdBar];
		m_selectedSongArc->add = 0.5;
		
		//Also have some particle systems; make them fire according to beat too
		for(int i = 0; i < m_selectedSongParticles.size(); i++)
		{
			if(m_selectedSongParticlesRateMul.size() > i && m_selectedSongParticlesThresh.size() > i && spec[i] >= m_selectedSongParticlesThresh[i])
				m_selectedSongParticles[i]->rate = m_selectedSongParticlesRateMul[i] * spec[i];
			else
				m_selectedSongParticles[i]->rate = 0;
		}
	}
	else
	{
		//First half of camera bounce; move back a bit every frame in an attempt to get back to default position
		if(CameraPos.z > m_fDefCameraZ)
			CameraPos.z -= m_fCamBounceBack;
		if(CameraPos.z < m_fDefCameraZ)
			CameraPos.z = m_fDefCameraZ;


		//Second half: test for threshold volume being exceeded and bounce camera forward
		if(spec[beatThresholdBar] >= beatThresholdVolume)
		{
			CameraPos.z += beatMul * spec[beatThresholdBar];
			if(CameraPos.z > m_fDefCameraZ + maxCamz)
				CameraPos.z = m_fDefCameraZ + maxCamz;
		}
	}
	
	delete [] spec;
	delete [] specLeft;
	delete [] specRight;
}

void Pony48Engine::loadSongXML(string sFilename)
{
	//Clean up old data
	cleanupSongGfx();
	
	XMLDocument* doc = new XMLDocument();
    int iErr = doc->LoadFile(sFilename.c_str());
	if(iErr != XML_NO_ERROR)
	{
		errlog << "Error parsing XML file " << sFilename << ": Error " << iErr << endl;
		delete doc;
		return;
	}

    XMLElement* root = doc->FirstChildElement("song");
    if(root == NULL)
	{
		errlog << "Error: No toplevel \"song\" item in XML file " << sFilename << endl;
		delete doc;
		return;
	}
	
	
	Rect rcCam = getCameraView();
	const char* cArtist = root->Attribute("artist");
	if(cArtist && strlen(cArtist))
	{
		HUDItem* hIt = m_hud->getChild("artist");
		if(hIt != NULL)
		{
			HUDTextbox* txt = (HUDTextbox*)hIt;
			string s = "by ";
			s += cArtist;
			txt->setText(s);
			txt->col.a = 1.0f;
			
			Point ptPos = txt->getPos();
			ptPos.x = rcCam.left + txt->getWidth() / 2.0f + 0.8f;
			txt->setPos(ptPos);
		}
	}
	const char* cTitle = root->Attribute("title");
	if(cTitle && strlen(cTitle))
	{
		HUDItem* hIt = m_hud->getChild("title");
		if(hIt != NULL)
		{
			HUDTextbox* txt = (HUDTextbox*)hIt;
			txt->setText(cTitle);
			txt->col.a = 1.0f;
			
			Point ptPos = txt->getPos();
			ptPos.x = rcCam.left + txt->getWidth() / 2.0f + 0.5f;
			txt->setPos(ptPos);
		}
	}
	m_fFadeoutTitleTime = getSeconds() + TITLE_DISPLAY_TIME;
	
	beatThresholdVolume = 0.75;
	beatThresholdBar = 0;
	beatMul = 45;
	maxCamz = 4;
	m_fCamBounceBack = 18.0;
	
	for(XMLElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		const char* cName = elem->Name();
		if(cName != NULL && strlen(cName))
		{
			string name = cName;
			if(name == "sfx")
			{
				const char* cPath = elem->Attribute("path");
				if(cPath != NULL && strlen(cPath))
					playMusic(cPath, m_fMusicVolume);
			}
			else if(name == "loop")
			{
				float32 start = -1;
				float32 end = -1;
				elem->QueryFloatAttribute("start", &start);
				elem->QueryFloatAttribute("end", &end);
				if(start > 0 && end > 0)
					musicLoop(start, end);
			}
			else if(name == "background")
			{
				const char* cBgType = elem->Attribute("type");
				if(cBgType != NULL && strlen(cBgType))
				{
					string sBgType = cBgType;
					if(sBgType == "pinwheel")
					{
						pinwheelBg* bg = new pinwheelBg();
						if(m_bg != NULL)
							delete m_bg;
						m_bg = (Background*) bg;
						list<Color> bgCols;
						for(XMLElement* spoke = elem->FirstChildElement("spoke"); spoke != NULL; spoke = spoke->NextSiblingElement("spoke"))
						{
							const char* cCol = spoke->Attribute("col");
							if(cCol != NULL && strlen(cCol))
								bgCols.push_back(colorFromString(cCol));
						}
						if(bgCols.size())
						{
							bg->init(bgCols.size());
							int cur = 0;
							for(list<Color>::iterator i = bgCols.begin(); i != bgCols.end(); i++, cur++)
								bg->setWheelCol(cur, *i);
						}
						elem->QueryFloatAttribute("speed", &bg->speed);
						elem->QueryFloatAttribute("rot", &bg->rot);
						elem->QueryFloatAttribute("acceleration", &bg->acceleration);
					}
					else if(sBgType == "starfield")
					{
						starfieldBg* bg = new starfieldBg();
						if(m_bg != NULL)
							delete m_bg;
						m_bg = (Background*) bg;
						elem->QueryFloatAttribute("speed", &bg->speed);
						const char* cCol = elem->Attribute("col");
						if(cCol && strlen(cCol))
							bg->col = colorFromString(cCol);
						elem->QueryUnsignedAttribute("num", &bg->num);
						const char* cSize = elem->Attribute("size");
						if(cSize && strlen(cSize))
							bg->fieldSize = vec3FromString(cSize);
						const char* cStarSize = elem->Attribute("starsize");
						if(cStarSize && strlen(cStarSize))
							bg->starSize = vec3FromString(cStarSize);
						const char* cAvoidCam = elem->Attribute("avoidcam");
						if(cAvoidCam && strlen(cAvoidCam))
							bg->avoidCam = pointFromString(cAvoidCam);
						bg->init();
					}
					//TODO
				}
			}
			else if(name == "bounce")
			{
				elem->QueryFloatAttribute("threshold", &beatThresholdVolume);
				elem->QueryUnsignedAttribute("bar", &beatThresholdBar);
				elem->QueryFloatAttribute("mul", &beatMul);
				elem->QueryFloatAttribute("max", &maxCamz);
				elem->QueryFloatAttribute("bounceback", &m_fCamBounceBack);
			}
			else if(name == "lua")
			{
				const char* cLuaFile = elem->Attribute("file");
				if(cLuaFile)
				{
					Lua->call("dofile", cLuaFile);
					const char* cLuaInitFunc = elem->Attribute("init");
					if(cLuaInitFunc)
						Lua->call(cLuaInitFunc);
					const char* cLuaUpdateFunc = elem->Attribute("update");
					if(cLuaUpdateFunc)
						sLuaUpdateFunc = cLuaUpdateFunc;
				}
			}
			else if(name == "particles")
			{
				const char* cParticleFilename = elem->Attribute("effect");
				const char* cParticleName = elem->Attribute("name");
				if(cParticleFilename && cParticleName)
				{
					ParticleSystem* pSys = new ParticleSystem();
					pSys->fromXML(cParticleFilename);
					pSys->init();
					songParticles[cParticleName] = pSys;
					elem->QueryBoolAttribute("autofire", &pSys->firing);
				}
			}
		}
	}
}

static float startedDecay = 0;

void Pony48Engine::scrubPause()
{
	startedDecay = getSeconds();
}

void Pony48Engine::scrubResume()
{
	startedDecay = -getSeconds();
}

const float timeToDecay = 0.75f;
static bool bPaused = false;

void Pony48Engine::soundUpdate(float32 dt)
{
	if(startedDecay < 0)	//Resuming
	{
		bPaused = false;
		float amt = soundFreqDefault / timeToDecay * dt;	//How much we should change by
		float freq = getMusicFrequency();
		freq += amt;
		if(freq >= soundFreqDefault)
		{
			freq = soundFreqDefault;
			startedDecay = 0;
		}
		setMusicFrequency(freq);
		dt *= freq / soundFreqDefault;
	}
	else if(startedDecay > 0)	//Pausing
	{
		bPaused = false;
		float amt = soundFreqDefault / timeToDecay * dt;	//How much we should change by
		float freq = getMusicFrequency();
		freq -= amt;
		if(freq <= 0)
		{
			freq = 0;
			startedDecay = 0;
			bPaused = true;
		}
		setMusicFrequency(freq);
		dt *= freq / soundFreqDefault;
	}
	
	if(!bPaused)
	{
		if(sLuaUpdateFunc.size())
			Lua->call(sLuaUpdateFunc.c_str(), getMusicPos());
		
		for(map<string, ParticleSystem*>::iterator i = songParticles.begin(); i != songParticles.end(); i++)
			i->second->update(dt);
		
		//Update background
		if(m_bg != NULL)
			m_bg->update(dt);
	}
}

void Pony48Engine::cleanupSongGfx()
{
	for(map<string, ParticleSystem*>::iterator i = songParticles.begin(); i != songParticles.end(); i++)
		delete (i->second);
	songParticles.clear();
	m_fSongFxRotate = 0.0f;
	if(m_bg != NULL)
		delete m_bg;
	m_bg = NULL;
	
	m_BoardBg.set(0.7,0.7,0.7,.5);
	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
			m_TileBg[j][i].set(0.5,0.5,0.5,.5);
	}
	m_BgCol.set(0,0,0,1.0);
	CameraPos.z = m_fDefCameraZ;
}




























