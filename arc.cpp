/*
    Pony48 source - arc.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "arc.h"

arc::arc(uint32_t number, Image* img) : physSegment()
{
	segmentPos = NULL;
	if(img == NULL || number == 0) return;
	numSegments = number;
	segmentPos = (float*)malloc(sizeof(float)*numSegments);
	arcSegImg = img;
	add = max = 0.0f;
	avg = 2;
	height = 0.1;
	p1.SetZero();
	p2.SetZero();
	init();
}

arc::~arc()
{
	free(segmentPos);
}

void arc::draw()
{
	if(!show || arcSegImg == NULL) return;
	glColor4f(col.r,col.g,col.b,col.a);
	glPushMatrix();
	
	//Calculate angle between two points and offset accordingly
	float32 fDistance = sqrt((p2.x-p1.x)*(p2.x-p1.x) + (p2.y-p1.y)*(p2.y-p1.y));	//Grahh slow
	float32 fAngle = -atan2((p2.y-p1.y),(p2.x-p1.x));
		
	//Offset according to depth
	glTranslatef(p1.x, -p1.y, depth);
	glRotatef(RAD2DEG*fAngle,0.0f,0.0f,1.0f);
	//Center on this point
	glTranslatef(0, -height / 2.0, 0);
	float32 fSegWidth = fDistance / (float32)(numSegments-1);
    for(int i = 0; i < numSegments-1; i++)
    {
      Point ul, ur, bl, br;
      ul.x = bl.x = (float32)i*fSegWidth;
      ur.x = br.x = bl.x + fSegWidth;
      ul.y = segmentPos[i];
      bl.y = ul.y + height;
      ur.y = segmentPos[i+1];
      br.y = ur.y + height;
	  arcSegImg->render4V(ul, ur, bl, br);
    }
	
	glPopMatrix();
	glColor4f(1.0,1.0,1.0,1.0);
}

void arc::update(float dt)
{
	dt *= 60.0;
	for(int i = 1; i < numSegments-1; i++)
	{
		segmentPos[i] += dt*randFloat(-add, add);
		if(segmentPos[i] > max)
			segmentPos[i] = max;
		if(segmentPos[i] < -max)
			segmentPos[i] = -max;
	}
	average();
}

void arc::init()
{
	//Initialize values of array to sane defaults, so we don't start with a flat arc for one frame
	for(int i = 0; i < numSegments; i++)
		segmentPos[i] = randFloat(-max, max);
	average();
}

void arc::average()
{
	if(avg < 1) return;
	
	float* temp = (float*) malloc(sizeof(float)*numSegments);
	
	//Copy our array values into temporary storage to work off of
	memcpy(temp, segmentPos, sizeof(float)*numSegments);
	
	//Center two end values
	temp[0] = temp[numSegments-1] = 0.0f;
	
	//Loop through, averaging values of all but two end values
	for(int i = 1; i < numSegments-1; i++)
    {
      float fTot = 0.0;
      for(int j = i-avg; j < i+avg+1; j++)
      {
		if(j >= 0 && j < numSegments)
			fTot += segmentPos[j];
	  }
      temp[i] = fTot / (float32)(avg*2+1);
    }
	
	//Copy back over
	memcpy(segmentPos, temp, sizeof(float)*numSegments);
	free(temp);
}









