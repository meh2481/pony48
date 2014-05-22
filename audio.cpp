/*
    Pony48 source - audio.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "Pony48.h"

void Pony48Engine::beatDetect()
{
	FMOD::Channel* channel = getChannel("music");
	if(channel == NULL) return;
	
	const int sampleSize = 64;
	
	//Code based off of http://katyscode.wordpress.com/2013/01/16/cutting-your-teeth-on-fmod-part-4-frequency-analysis-graphic-equalizer-beat-detection-and-bpm-estimation/
	float *specLeft, *specRight;

	specLeft = new float[sampleSize];
	specRight = new float[sampleSize];

	// Get spectrum for left and right stereo channels
	channel->getSpectrum(specLeft, sampleSize, 0, FMOD_DSP_FFT_WINDOW_RECT);
	channel->getSpectrum(specRight, sampleSize, 1, FMOD_DSP_FFT_WINDOW_RECT);
	float *spec;

	spec = new float[sampleSize];

	for (int i = 0; i < sampleSize; i++)
		spec[i] = (specLeft[i] + specRight[i]) / 2;
	
	float beatThresholdVolume = 0.75f;    // The threshold over which to recognize a beat
	int beatThresholdBar = 0;            // The bar in the volume distribution to examine
	const unsigned int beatPostIgnore = 250;   // Number of ms to ignore track for after a beat is recognized

	static int beatLastTick = 0;                // Time when last beat occurred
	
	bool beatDetected = false;
	
	//Print out a sort of 
	/*printf("\033[2J\033[1;1H");
	for(int i = 0; i < 10; i++)
	{
		//if(spec[i] < 0.8) continue;
		int num = spec[i] * 20;
		for(int j = 0; j < num; j++)
			printf("*");
		for(int rest = num; rest < 20; rest++)
			printf(" ");
		printf("|\n");
	}*/

	// Test for threshold volume being exceeded (if not currently ignoring track)
	if (spec[beatThresholdBar] >= beatThresholdVolume)
	{
		//cout << "wub " << spec[beatThresholdBar] << endl;
		CameraPos.z += 0.75 * spec[beatThresholdBar];
		if(CameraPos.z > m_fDefCameraZ + 4)
			CameraPos.z = m_fDefCameraZ + 4;
	}

	/*if (beatDetected)
	{
		// A beat has occurred, do something here
		cout << "wub " << spec[beatThresholdBar] << endl;
		//printf("\033[2J\033[1;1H");
	}

	// If the ignore time has expired, allow testing for a beat again
	if (SDL_GetTicks() - beatLastTick >= beatPostIgnore)
		beatLastTick = 0;*/
}

































