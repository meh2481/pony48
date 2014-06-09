/*
    Pony48 source - webcam.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "webcam.h"

Webcam::Webcam()
{
	m_VideoCap = NULL;
	m_curFrame = NULL;
	m_hTex = 0;
	m_iWidth = m_iHeight = -1;
}

Webcam::~Webcam()
{
	_clear();
}

void Webcam::draw(float32 height, Point ptCenter)
{
	if(m_hTex)
	{
		// tell opengl to use the generated texture
		glBindTexture(GL_TEXTURE_2D, m_hTex);
		
		float32 width = height * ((float32)m_iWidth / (float32)m_iHeight);
		width /= 2.0;
		height /= 2.0;
		
		const GLfloat vertexData[] =
		{
			ptCenter.x - width, ptCenter.y + height, // upper left
			ptCenter.x + width, ptCenter.y + height, // upper right
			ptCenter.x - width, ptCenter.y - height, // lower left
			ptCenter.x + width, ptCenter.y - height, // lower right
		};
		const GLfloat texCoords[] =
		{
			0, 0, // upper left
			1, 0, // upper right
			0, 1, // lower left
			1, 1, // lower right
		};
		glVertexPointer(2, GL_FLOAT, 0, &vertexData);
		glTexCoordPointer(2, GL_FLOAT, 0, &texCoords);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

void Webcam::getNewFrame()
{
	if(m_VideoCap == NULL || !m_VideoCap->isOpened())
		return;
	if(m_curFrame == NULL)
		m_curFrame = new cv::Mat;
	if(m_VideoCap->read(*m_curFrame))	//Grab frame from webcam (can take some time)
	{
		m_iHeight = m_curFrame->rows;
		m_iWidth = m_curFrame->cols;
		if(m_iHeight > 0 && m_iWidth > 0)
		{
			//Create OpenGL texture out of this data
			if(!m_hTex)
				glGenTextures(1, &m_hTex);
			glBindTexture(GL_TEXTURE_2D, m_hTex);
			
			int mode, modeflip;
#ifdef __BIG_ENDIAN__	//TODO: Test
			mode = GL_RGB;
			modeflip = GL_RGB;
#else
			mode = GL_RGB;
			modeflip = GL_BGR;
#endif
			glTexImage2D(GL_TEXTURE_2D, 0, mode, m_iWidth, m_iHeight, 0, modeflip, GL_UNSIGNED_BYTE, m_curFrame->data);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		}
	}
}

void Webcam::open(int device)
{
	_clear();
	//Open the webcam with OpenCV
	m_VideoCap = new cv::VideoCapture(device);
	if(!m_VideoCap->isOpened())	
	{
		errlog << "Unable to open webcam " << device << endl;
		delete m_VideoCap;
		m_VideoCap = NULL;
	}
}

void Webcam::_clear()
{
	if(m_VideoCap)
		delete m_VideoCap;
	m_VideoCap = NULL;
	if(m_curFrame)
		delete m_curFrame;
	m_curFrame = NULL;
	if(m_hTex)
		glDeleteTextures(1, &m_hTex);
	m_hTex = 0;
	m_iWidth = m_iHeight = -1;
}

bool Webcam::isOpen()
{
	return (m_VideoCap != NULL && m_VideoCap->isOpened());
}

