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

	for (int i = 0; i < sampleSize; i++)
		spec[i] = (specLeft[i] + specRight[i]) / 2.0;
	
	float beatThresholdVolume = 0.75f;    // The threshold over which to recognize a beat
	int beatThresholdBar = 0;            // The bar in the volume distribution to examine

	// Test for threshold volume being exceeded , and bounce camera
	if (spec[beatThresholdBar] >= beatThresholdVolume)
	{
		CameraPos.z += 0.75 * spec[beatThresholdBar];
		if(CameraPos.z > m_fDefCameraZ + 4)
			CameraPos.z = m_fDefCameraZ + 4;
	}
	
	delete [] spec;
	delete [] specLeft;
	delete [] specRight;
}

void Pony48Engine::loadSongs(string sFilename)
{
	
}

static float startedDecay = 0;

void Pony48Engine::scrubPause()
{
	//pauseMusic();
	startedDecay = getSeconds();
}

void Pony48Engine::scrubResume()
{
	//resumeMusic();
	startedDecay = -getSeconds();
}

const float soundFreqDefault = 44100.0;
const float timeToDecay = 0.5f;

void Pony48Engine::soundUpdate(float32 dt)
{
	if(startedDecay < 0)	//Resuming
	{
		FMOD::Channel* channel = getChannel("music");
		if(channel != NULL)
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
	}
	else if(startedDecay > 0)	//Pausing
	{
		FMOD::Channel* channel = getChannel("music");
		
		if(channel != NULL)
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
	}
}






























