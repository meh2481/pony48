/*
    Pony48 source - bg.cpp
    Copyright (c) 2014 Mark Hutcheson
*/
#include "bg.h"

//-----------------------------------------------------------------------------
// Pinwheel background functions
//-----------------------------------------------------------------------------

pinwheelBg::pinwheelBg()
{
	m_iNumSpokes = 0;
	screenDiag = 1;
	acceleration = speed = rot = 0;
	m_lWheel = NULL;
}

pinwheelBg::~pinwheelBg()
{
	if(m_lWheel != NULL)
		delete [] m_lWheel;
}

void pinwheelBg::draw()
{
	if(m_lWheel == NULL || !m_iNumSpokes) return;
	float32 addAngle = 360.0 / m_iNumSpokes;
	glPushMatrix();
	glRotatef(rot, 0, 0, 1);	//Rotate according to current rotation
	for(int i = 0; i < m_iNumSpokes; i++)
	{
		glBegin(GL_TRIANGLES);
		glColor4f(m_lWheel[i].r, m_lWheel[i].g, m_lWheel[i].b, m_lWheel[i].a);
		glVertex3f(0, 0, 0);	//Center pt
		glVertex3f(-screenDiag, 0, 0);	//Leftmost pt
		glVertex3f(-cos(DEG2RAD*addAngle)*screenDiag, sin(DEG2RAD*addAngle)*screenDiag, 0);	//Upper left pt
		glEnd();
		glRotatef(addAngle, 0, 0, 1);	//Rotate for next segment
	}
	glPopMatrix();
		
}

void pinwheelBg::update(float32 dt)
{
	rot += speed * dt;
	speed += acceleration * dt;
}

void pinwheelBg::init(uint32_t num)
{
	if(num)
	{
		m_iNumSpokes = num;
		if(m_lWheel != NULL)
			delete [] m_lWheel;
		m_lWheel = new Color[num];
	}
}

void pinwheelBg::setWheelCol(uint32_t wheel, Color col)
{
	if(m_iNumSpokes <= wheel) return;
	if(m_lWheel == NULL) return;
	m_lWheel[wheel] = col;
}

Color* pinwheelBg::getWheelCol(uint32_t wheel)
{
	if(m_iNumSpokes <= wheel)
		return NULL;
	return &m_lWheel[wheel];
}

//-----------------------------------------------------------------------------
// Starfield background functions
//-----------------------------------------------------------------------------
starfieldBg::starfieldBg()
{
	bg.set(0,0,0,1);
	gen.set(1,1,1,1);
	acceleration = 0.01;
	speed = 1;
	num = 100;
	centerSize.Set(0.1,0.1);
	edgeSize.Set(0.3,0.3);
}

void starfieldBg::init()
{
	//TODO
}

void starfieldBg::draw()
{
	//Fill in bg color
	glColor4f(bg.r, bg.g, bg.b, bg.a);
	glBindTexture(GL_TEXTURE_2D, 0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glBegin(GL_QUADS);
	glVertex3i(-1, -1, -1);
	glVertex3i(1, -1, -1);
	glVertex3i(1, 1, -1);
	glVertex3i(-1, 1, -1);
	glEnd();
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glColor4f(1,1,1,1);
	
	//Draw stars
	for(list<starfieldStar>::iterator i = m_lStars.begin(); i != m_lStars.end(); i++)
	{
		//TODO
	}
}

void starfieldBg::update(float32 dt)
{
	return;//TODO
	while(m_lStars.size() < num)
	{
		starfieldStar st;
		//TODO
	}
}


//-----------------------------------------------------------------------------
// Gradient background functions
//-----------------------------------------------------------------------------

void gradientBg::draw()
{
	//Fill whole screen with rect (Example taken from http://yuhasapoint.blogspot.com/2012/07/draw-quad-that-fills-entire-opengl.html on 11/20/13)
	glBindTexture(GL_TEXTURE_2D, 0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glBegin(GL_QUADS);
	glColor4f(bl.r, bl.g, bl.b, bl.a);
	glVertex3i(-1, -1, -1);
	glColor4f(br.r, br.g, br.b, br.a);
	glVertex3i(1, -1, -1);
	glColor4f(ur.r, ur.g, ur.b, ur.a);
	glVertex3i(1, 1, -1);
	glColor4f(ul.r, ul.g, ul.b, ul.a);
	glVertex3i(-1, 1, -1);
	glEnd();
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}






































