/*
    Pony48 source - particles.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "particles.h"

//Additive
//glBlendFunc(GL_DST_COLOR, GL_ONE);

//Normal
//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//Subtractivey sorta thing?
//glBlendFunc(GL_ZERO, GL_SRC_ALPHA); 

ParticleSystem::ParticleSystem()
{
	m_imgRect = NULL;
	m_pos = NULL;
	m_sizeStart = NULL;
	m_sizeEnd = NULL;
	m_vel = NULL;
	m_accel = NULL;
	m_rot = NULL;
	m_rotVel = NULL;
	m_rotAccel = NULL;
	m_colStart = NULL;
	m_colEnd = NULL;
	m_tangentialAccel = NULL;
	m_normalAccel = NULL;
	m_lifetime = NULL;
	m_created = NULL;
	m_num = 0;
	
	sizeStart = Point(1,1);
	sizeEnd = Point(1,1);
	sizeVar = Point(0,0);
	speed = 1.0;
	speedVar = 0.0;
	accel = Point(0,0);
	accelVar = Point(0,0);
	rotStart = 0.0;
	rotStartVar = 0.0;
	rotVel = 0.0;
	rotVelVar = 0.0;
	rotAccel = 0.0;
	rotAccelVar = 0.0;
	colStart = Color(1,1,1,1);
	colEnd = Color(1,1,1,1);
	colVar = Color(0,0,0,0);
	tangentialAccel = 0;
	tangentialAccelVar = 0;
	normalAccel = 0;
	normalAccelVar = 0;
	lifetime = 4;
	lifetimeVar = 0;
	
	img = NULL;
	max = 100;
	rate = 25;
	emitFrom = Rect(0,0,0,0);
	blend = ADDITIVE;
	emissionAngle = 0;
	emissionAngleVar = 0;
}

ParticleSystem::~ParticleSystem()
{
	if(m_imgRect != NULL)
		delete m_imgRect;
	if(m_pos != NULL)
		delete m_pos;
	if(m_sizeStart != NULL)
		delete m_sizeStart;
	if(m_sizeEnd != NULL)
		delete m_sizeEnd;
	if(m_vel != NULL)
		delete m_vel;
	if(m_accel != NULL)
		delete m_accel;
	if(m_rot != NULL)
		delete m_rot;
	if(m_rotVel != NULL)
		delete m_rotVel;
	if(m_rotAccel != NULL)
		delete m_rotAccel;
	if(m_colStart != NULL)
		delete m_colStart;
	if(m_colEnd != NULL)
		delete m_colEnd;
	if(m_tangentialAccel != NULL)
		delete m_tangentialAccel;
	if(m_normalAccel != NULL)
		delete m_normalAccel;
	if(m_lifetime != NULL)
		delete m_lifetime;
	if(m_created != NULL)
		delete m_created;
}

void ParticleSystem::update(float32 dt)
{
	
}

void ParticleSystem::draw()
{
	
}

void ParticleSystem::init()
{
	
}









































