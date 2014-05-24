/*
    Pony48 source - audio.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "Pony48.h"
const int sampleSize = 64;

void Pony48Engine::beatDetect()
{
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































