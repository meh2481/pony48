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
	gen.set(1,1,1,1);
	speed = 15;
	num = 500;
	fieldSize.set(40,40,75);
	starSize.Set(0.1, 0.1);
}

void starfieldBg::init()
{
	for(int i = 0; i < num; i++)
	{
		starfieldStar st;
		st.col = gen;
		st.size = starSize;
		st.pos.z = randFloat(0,fieldSize.z);
		st.pos.x = randFloat(-fieldSize.x/2.0, fieldSize.x/2.0);
		st.pos.y = randFloat(-fieldSize.y/2.0, fieldSize.y/2.0);
		m_lStars.push_back(st);
	}
}

void starfieldBg::draw()
{	
	glPushMatrix();
	glLoadIdentity();	//So camera is at z = 0
	//Draw stars
	for(list<starfieldStar>::iterator i = m_lStars.begin(); i != m_lStars.end(); i++)
	{
		glColor4f(i->col.r,i->col.g,i->col.b,i->col.a);
		glPushMatrix();
		glTranslatef(i->pos.x, i->pos.y, -i->pos.z);
		glBegin(GL_QUADS);
		glVertex3f(-i->size.x/2.0, i->size.y/2.0, 0);
		glVertex3f(i->size.x/2.0, i->size.y/2.0, 0);
		glVertex3f(i->size.x/2.0, -i->size.y/2.0, 0);
		glVertex3f(-i->size.x/2.0, -i->size.y/2.0, 0);
		glEnd();
		glPopMatrix();
	}
	glColor4f(1,1,1,1);
	glPopMatrix();
}

void starfieldBg::update(float32 dt)
{
	//Update all the stars here
	for(list<starfieldStar>::iterator i = m_lStars.begin(); i != m_lStars.end(); i++)
	{
		i->pos.z -= speed*dt;	//Update position
		if(i->pos.z <= 0)	//If this particle has gone off the screen
		{
			i->col = gen;
			i->size = starSize;
			i->pos.z = fieldSize.z;
			i->pos.x = randFloat(-fieldSize.x/2.0, fieldSize.x/2.0);
			i->pos.y = randFloat(-fieldSize.y/2.0, fieldSize.y/2.0);
		}
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






































