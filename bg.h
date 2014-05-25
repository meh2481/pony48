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
	Background(){screenDiag = 1;};
	~Background(){};
	
	float32 screenDiag;
	
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
	
	void init();	//Place all stars randomly to begin with
	
	Color gen;		//Starting color
	float32 speed;	//Simulation speed
	uint32_t num;	//Maximum number of stars
	Point starSize;	//Size of stars
	Vec3 fieldSize;	//How large the starfield is
	
protected:
	class starfieldStar
	{
	public:
		Color col;
		Vec3 pos;
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






































