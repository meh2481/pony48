/*
    Pony48 source - particles.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "particles.h"

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
	firing = true;
	
	curTime = 0;
	spawnCounter = 0;
}

ParticleSystem::~ParticleSystem()
{
	_deleteAll();
}

void ParticleSystem::_deleteAll()
{
	if(m_imgRect != NULL)
		delete [] m_imgRect;
	if(m_pos != NULL)
		delete [] m_pos;
	if(m_sizeStart != NULL)
		delete [] m_sizeStart;
	if(m_sizeEnd != NULL)
		delete [] m_sizeEnd;
	if(m_vel != NULL)
		delete [] m_vel;
	if(m_accel != NULL)
		delete [] m_accel;
	if(m_rot != NULL)
		delete [] m_rot;
	if(m_rotVel != NULL)
		delete [] m_rotVel;
	if(m_rotAccel != NULL)
		delete [] m_rotAccel;
	if(m_colStart != NULL)
		delete [] m_colStart;
	if(m_colEnd != NULL)
		delete [] m_colEnd;
	if(m_tangentialAccel != NULL)
		delete [] m_tangentialAccel;
	if(m_normalAccel != NULL)
		delete [] m_normalAccel;
	if(m_lifetime != NULL)
		delete [] m_lifetime;
	if(m_created != NULL)
		delete [] m_created;
	
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
}

void ParticleSystem::_newParticle()
{
	if(m_num == max) return;	//Don't create more particles than we can!
	if(!firing) return;
	
	if(!imgRect.size())
	{
		if(img != NULL)
			m_imgRect[m_num] = Rect(0,0,img->getWidth(),img->getHeight());
		else
			m_imgRect[m_num] = Rect(0,0,0,0);
	}
	else
		m_imgRect[m_num] = imgRect[randInt(0, imgRect.size()-1)];
	m_pos[m_num] = Point(randFloat(-emitFrom.center().x-emitFrom.width()/2.0,emitFrom.center().x+emitFrom.width()/2.0)+emitFrom.center().x, 
						 randFloat(-emitFrom.center().y-emitFrom.height()/2.0,emitFrom.center().y+emitFrom.height()/2.0)+emitFrom.center().y);
	m_sizeStart[m_num].x = sizeStart.x + randFloat(-sizeVar.x,sizeVar.x);
	m_sizeStart[m_num].y = sizeStart.y + randFloat(-sizeVar.y,sizeVar.y);
	m_sizeEnd[m_num].x = sizeEnd.x + randFloat(-sizeVar.x,sizeVar.x);
	m_sizeEnd[m_num].y = sizeEnd.y + randFloat(-sizeVar.y,sizeVar.y);
	float32 angle = emissionAngle + randFloat(-emissionAngleVar,emissionAngleVar);
	float32 amt = speed + randFloat(-speedVar,speedVar);
	m_vel[m_num].x = amt*cos(DEG2RAD*angle);
	m_vel[m_num].y = amt*sin(DEG2RAD*angle);
	m_accel[m_num].x = accel.x + randFloat(-accelVar.x,accelVar.x);
	m_accel[m_num].y = accel.y + randFloat(-accelVar.y,accelVar.y);
	m_rot[m_num] = rotStart + randFloat(-rotStartVar,rotStartVar);
	m_rotVel[m_num] = rotVel + randFloat(-rotVelVar,rotVelVar);
	m_rotAccel[m_num] = rotAccel + randFloat(-rotAccelVar,rotAccelVar);
	m_colStart[m_num].r = colStart.r + randFloat(-colVar.r,colVar.r);
	m_colStart[m_num].g = colStart.g + randFloat(-colVar.g,colVar.g);
	m_colStart[m_num].b = colStart.b + randFloat(-colVar.b,colVar.b);
	m_colStart[m_num].a = colStart.a + randFloat(-colVar.a,colVar.a);
	m_colEnd[m_num].r = colEnd.r + randFloat(-colVar.r,colVar.r);
	m_colEnd[m_num].g = colEnd.g + randFloat(-colVar.g,colVar.g);
	m_colEnd[m_num].b = colEnd.b + randFloat(-colVar.b,colVar.b);
	m_colEnd[m_num].a = colEnd.a + randFloat(-colVar.a,colVar.a);
	m_tangentialAccel[m_num] = tangentialAccel + randFloat(-tangentialAccelVar,tangentialAccelVar);
	m_normalAccel[m_num] = normalAccel + randFloat(-normalAccelVar,normalAccelVar);
	m_lifetime[m_num] = lifetime + randFloat(-lifetimeVar,lifetimeVar);
	m_created[m_num] = curTime;
	
	m_num++;
}

void ParticleSystem::_rmParticle(uint32_t idx)
{
	//Order doesn't matter, so just shift the newest particle over to replace this one
	m_imgRect[idx] = m_imgRect[m_num-1];
	m_pos[idx] = m_pos[m_num-1];
	m_sizeStart[idx] = m_sizeStart[m_num-1];
	m_sizeEnd[idx] = m_sizeEnd[m_num-1];
	m_vel[idx] = m_vel[m_num-1];
	m_accel[idx] = m_accel[m_num-1];
	m_rot[idx] = m_rot[m_num-1];
	m_rotVel[idx] = m_rotVel[m_num-1];
	m_rotAccel[idx] = m_rotAccel[m_num-1];
	m_colStart[idx] = m_colStart[m_num-1];
	m_colEnd[idx] = m_colEnd[m_num-1];
	m_tangentialAccel[idx] = m_tangentialAccel[m_num-1];
	m_normalAccel[idx] = m_normalAccel[m_num-1];
	m_lifetime[idx] = m_lifetime[m_num-1];
	m_created[idx] = m_created[m_num-1];
	
	m_num--;
}

void ParticleSystem::update(float32 dt)
{
	curTime += dt;
	
	spawnCounter += dt * rate;
	int iSpawnAmt = floor(spawnCounter);
	spawnCounter -= iSpawnAmt;
	for(int i = 0; i < iSpawnAmt; i++)
		_newParticle();
	
	//Update particle fields (In separate for loops to cut down on cache thrashing)
	for(int i = 0; i < m_num; i++)
	{
		m_pos[i].x += m_vel[i].x * dt;
		m_pos[i].y += m_vel[i].y * dt;
	}
	
	for(int i = 0; i < m_num; i++)
	{
		m_vel[i].x += m_accel[i].x * dt;
		m_vel[i].y += m_accel[i].y * dt;
		//TODO Deal with these later
		//m_tangentialAccel
		//m_normalAccel
	}
	
	for(int i = 0; i < m_num; i++)
		m_rot[i] += m_rotVel[i] * dt;
	
	for(int i = 0; i < m_num; i++)
		m_rotVel[i] += m_rotAccel[i] * dt;
	
	for(int i = 0; i < m_num; i++)
	{
		if(curTime - m_created[i] > m_lifetime[i])	//time for this particle go bye-bye
		{
			_rmParticle(i);
			i--;	//Go back a particle so we don't skip anything
		}
	}
}

void ParticleSystem::draw()
{
	if(img == NULL) return;
	
	switch(blend)
	{
		case ADDITIVE:
			glBlendFunc(GL_DST_COLOR, GL_ONE);
			break;
			
		case NORMAL:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
			
		case SUBTRACTIVE:
			glBlendFunc(GL_ZERO, GL_SRC_ALPHA); 
			break;
	}
	
	for(int i = 0; i < m_num; i++)	//Can't really help cache-thrashing here, so do it all in one loop
	{
		float32 fLifeFac = (curTime - m_created[i]) / m_lifetime[i];
		if(fLifeFac > 1.0) continue;
		Color drawcol;
		Point drawsz;
		drawcol.r = (m_colEnd[i].r - m_colStart[i].r) * fLifeFac + m_colStart[i].r;
		drawcol.g = (m_colEnd[i].g - m_colStart[i].g) * fLifeFac + m_colStart[i].g;
		drawcol.b = (m_colEnd[i].b - m_colStart[i].b) * fLifeFac + m_colStart[i].b;
		drawcol.a = (m_colEnd[i].a - m_colStart[i].a) * fLifeFac + m_colStart[i].a;
		drawsz.x = (m_sizeEnd[i].x - m_sizeStart[i].x) * fLifeFac + m_sizeStart[i].x;
		drawsz.y = (m_sizeEnd[i].y - m_sizeStart[i].y) * fLifeFac + m_sizeStart[i].y;
		glPushMatrix();
		glColor4f(drawcol.r, drawcol.g, drawcol.b, drawcol.a);
		glTranslatef(m_pos[i].x, m_pos[i].y, 0);
		glRotatef(m_rot[i], 0, 0, 1);
		img->render(drawsz, m_imgRect[i]);
		glPopMatrix();
	}
	
	//Reset OpenGL stuff
	glColor4f(1,1,1,1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleSystem::init()
{
	if(m_num)
		_deleteAll();
	
	if(!max) return;
	
	m_imgRect = new Rect[max];
	m_pos = new Point[max];
	m_sizeStart = new Point[max];
	m_sizeEnd = new Point[max];
	m_vel = new Point[max];
	m_accel = new Point[max];
	m_rot = new float32[max];
	m_rotVel = new float32[max];
	m_rotAccel = new float32[max];
	m_colStart = new Color[max];
	m_colEnd = new Color[max];
	m_tangentialAccel = new float32[max];
	m_normalAccel = new float32[max];
	m_lifetime = new float32[max];
	m_created = new float32[max];
}

void ParticleSystem::fromXML(string sXMLFilename)
{
	
}







































