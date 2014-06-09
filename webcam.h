/*
    Pony48 header - webcam.h
    Copyright (c) 2014 Mark Hutcheson
*/

#ifndef WEBCAM_H
#define WEBCAM_H

#include "opencv2/opencv.hpp"
#include "globaldefs.h"

class Webcam
{
protected:
	cv::VideoCapture*	m_VideoCap;
	cv::Mat* 			m_curFrame;
	GLuint   			m_hTex;
	
	int m_iWidth, m_iHeight;
	
	void _clear();	//Clear memory associated with webcam objects
	
public:
	Webcam();
	~Webcam();
	
	void draw(float32 height, Point ptCenter);			//Draw the previous frame to the screen, with the specified height (width is determined by frame width)
	void getNewFrame();		//Get a new frame from the webcam
	int getFrameWidth() 	{return m_iWidth;};		//Get width of a webcam frame
	int getFrameHeight()	{return m_iHeight;};	//Get height of a webcam frame
	
	void open(int device);			//Open specified webcam
	bool isOpen();
	
	bool mirror;					//If we should mirror-image this when drawing or not
};






































#endif	//defined WEBCAM_H
 
