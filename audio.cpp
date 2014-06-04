/*
    Pony48 source - audio.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "Pony48.h"

const int sampleSize = 64;

void Pony48Engine::beatDetect()
{
	if(m_iCurMode == GAMEOVER) return;	//music stops when game is over, so it looks silly
	
	FMOD::Channel* channel = getChannel("music");
	if(channel == NULL) return;
	
	
	//Code based off of http://katyscode.wordpress.com/2013/01/16/cutting-your-teeth-on-fmod-part-4-frequency-analysis-graphic-equalizer-beat-detection-and-bpm-estimation/
	float *specLeft, *specRight, *spec;

	specLeft = new float[sampleSize];
	specRight = new float[sampleSize];
	spec = new float[sampleSize];

	// Get spectrum for left and right stereo channels
	channel->getSpectrum(specLeft, sampleSize, 0, FMOD_DSP_FFT_WINDOW_RECT);
	channel->getSpectrum(specRight, sampleSize, 1, FMOD_DSP_FFT_WINDOW_RECT);

	//Center for a mono sound
	for(int i = 0; i < sampleSize; i++)
		spec[i] = (specLeft[i] + specRight[i]) / 2.0;
	
#ifdef DEBUG
	//Print out a sort of audio-level thing
	/*printf("\033[2J\033[1;1H");
	for(int i = 0; i < 10; i++)
	{
		int num = spec[i] * 20;
		for(int j = 0; j < num; j++)
			printf("*");
		for(int rest = num; rest < 20; rest++)
			printf(" ");
		printf("|\n");
	}*/
#endif


	//Test for threshold volume being exceeded and bounce camera
	if(spec[beatThresholdBar] >= beatThresholdVolume)
	{
		CameraPos.z += beatMul * spec[beatThresholdBar];
		if(CameraPos.z > m_fDefCameraZ + maxCamz)
			CameraPos.z = m_fDefCameraZ + maxCamz;
	}
	
	delete [] spec;
	delete [] specLeft;
	delete [] specRight;
}

void Pony48Engine::loadSongXML(string sFilename)
{
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
					playMusic(cPath);
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
					//TODO
				}
			}
			else if(name == "bounce")
			{
				elem->QueryFloatAttribute("threshold", &beatThresholdVolume);
				elem->QueryUnsignedAttribute("bar", &beatThresholdBar);
				elem->QueryFloatAttribute("mul", &beatMul);
				elem->QueryFloatAttribute("max", &maxCamz);
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

void Pony48Engine::loadSongs(string sFilename)
{
	XMLDocument* doc = new XMLDocument();
    int iErr = doc->LoadFile(sFilename.c_str());
	if(iErr != XML_NO_ERROR)
	{
		errlog << "Error parsing XML file " << sFilename << ": Error " << iErr << endl;
		delete doc;
		return;
	}

    XMLElement* root = doc->FirstChildElement("music");
    if(root == NULL)
	{
		errlog << "Error: No toplevel \"music\" item in XML file " << sFilename << endl;
		delete doc;
		return;
	}
	
	//TODO Song select
	for(XMLElement* song = root->FirstChildElement("song"); song != NULL; song = song->NextSiblingElement("song"))
	{
		const char* cPath = song->Attribute("path");
		if(cPath != NULL && strlen(cPath))
		{
			loadSongXML(cPath);
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

const float timeToDecay = 0.5f;

void Pony48Engine::soundUpdate(float32 dt)
{
	FMOD::Channel* channel = getChannel("music");
	if(channel != NULL)
	{
		if(startedDecay < 0)	//Resuming
		{
			float amt = soundFreqDefault / timeToDecay * dt;	//How much we should change by
			float freq;
			channel->getFrequency(&freq);
			freq += amt;
			if(freq >= soundFreqDefault)
			{
				freq = soundFreqDefault;
				startedDecay = 0;
			}
			channel->setFrequency(freq);
		}
		else if(startedDecay > 0)	//Pausing
		{
			float amt = soundFreqDefault / timeToDecay * dt;	//How much we should change by
			float freq;
			channel->getFrequency(&freq);
			freq -= amt;
			if(freq <= 0)
			{
				freq = 0;
				startedDecay = 0;
			}
			channel->setFrequency(freq);
		}
#ifdef DEBUG
		else if(keyDown(SDL_SCANCODE_2))
			channel->setFrequency(soundFreqDefault*2);
		else if(keyDown(SDL_SCANCODE_3))
			channel->setFrequency(soundFreqDefault*3);
		else if(keyDown(SDL_SCANCODE_4))
			channel->setFrequency(soundFreqDefault*4);
		else if(keyDown(SDL_SCANCODE_5))
			channel->setFrequency(soundFreqDefault*5);
	#ifdef DEBUG_REVSOUND
		else if(keyDown(SDL_SCANCODE_6))
			channel->setFrequency(soundFreqDefault*-1);
		else if(keyDown(SDL_SCANCODE_7))
			channel->setFrequency(soundFreqDefault*-2);
		else if(keyDown(SDL_SCANCODE_8))
			channel->setFrequency(soundFreqDefault*-3);
		else if(keyDown(SDL_SCANCODE_9))
			channel->setFrequency(soundFreqDefault*-4);
		else if(keyDown(SDL_SCANCODE_0))
			channel->setFrequency(soundFreqDefault*-5);
	#endif
		//else if(m_iCurMode != GAMEOVER)
		//	channel->setFrequency(soundFreqDefault);	//Reset to playing normally
#endif
		if(sLuaUpdateFunc.size())
		{
			unsigned int ms;
			channel->getPosition(&ms, FMOD_TIMEUNIT_MS);
			Lua->call(sLuaUpdateFunc.c_str(), (float)ms/1000.0);
		}
	}
	for(map<string, ParticleSystem*>::iterator i = songParticles.begin(); i != songParticles.end(); i++)
		i->second->update(dt);
}






























