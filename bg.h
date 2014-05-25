/*
    Pony48 header - bg.h
    Defines classes for different background types
    Copyright (c) 2014 Mark Hutcheson
*/
#include "globaldefs.h"

//Background base class
class Background
{
public:
	Background(){};
	~Background(){};
	
	virtual void draw() = 0;
	virtual void update(float32 dt) = 0;
};

class pinwheelBg : public Background
{
public:
	pinwheelBg();
	~pinwheelBg();
	
	void draw();
	void update(float32 dt);
	
	void init(uint32_t num);
	uint32_t getNum(){return m_iNumSpokes;};
	void setWheelCol(uint32_t wheel, Color col);
	Color* getWheelCol(uint32_t wheel);
	
	float32 speed;
	float32 rot;
	float32 acceleration;
	float32 screenDiag;	//How large the screen is diagonally, from corner to corner
	
protected:
	Color* m_lWheel;
	uint32_t m_iNumSpokes;
};

class starfieldBg : public Background
{
public:
	starfieldBg();
	~starfieldBg(){};
	
	void draw();
	void update(float32 dt);
	
	void init();	//Creates stars already there, so it doesn't start with a blank screen
	
	Color bg;			//Background color
	Color gen;		//Starting color
	float32 acceleration;
	float32 speed;	//Starting speed
	uint32_t num;	//Maximum number of stars
	Point centerSize;	//Size of the image at center of screen
	Point edgeSize;		//Size of the image at edge of screen
	
protected:
	class starfieldStar
	{
	public:
		Point velocity;
		Color col;
		Point pos;
		Point size;
	};

	list<starfieldStar> m_lStars;
};

class gradientBg : public Background
{
public:
	gradientBg(){};
	~gradientBg(){};
	
	void draw();
	void update(float32 dt){};
	
	Color ul, ur, bl, br;
};






































