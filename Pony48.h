/*
    Pony48 source - Pony48.h
    Copyright (c) 2014 Mark Hutcheson
*/
#ifndef PONY48ENGINE_H
#define PONY48ENGINE_H

#include "Engine.h"
#include <vector>
#include <set>

#define DEBUG	//Debug mode; cheat keys and such

#define DEFAULT_WIDTH	800
#define DEFAULT_HEIGHT	600

#define DEFAULT_TIMESCALE	1.0

//const variables

class ColorPhase
{
public:
	Color* colorToChange;
	bool pingpong;
	float32 destr, destg, destb;
	float32 srcr, srcg, srcb;
	float32 amtr, amtg, amtb;
	bool dir;
};

class Pony48Engine : public Engine
{
private:
  //Important general-purpose game variables
  ttvfs::VFSHelper vfs;
  Vec3 CameraPos;
  HUD* m_hud;
  bool m_bMouseGrabOnWindowRegain;
  float32 m_fDefCameraZ;	//Default position of camera on z axis
  list<ColorPhase> m_ColorsChanging;
  
  //Game stuff!
	

protected:
    void frame(float32 dt);
    void draw();
    void init(list<commandlineArg> sArgs);
    void handleEvent(SDL_Event event);
	void pause();
	void resume();

public:
	//Pony48.cpp functions - fairly generic 
    Pony48Engine(uint16_t iWidth, uint16_t iHeight, string sTitle, string sAppName, string sIcon, bool bResizable = false);
    ~Pony48Engine();
	
	bool _shouldSelect(b2Fixture* fix);

    void hudSignalHandler(string sSignal);  //For handling signals that come from the HUD
	void handleKeys();						//Poll the keyboard state and update the game accordingly
	Point worldPosFromCursor(Point cursorpos);	//Get the worldspace position of the given mouse cursor position
	Point worldMovement(Point cursormove);		//Get the worldspace transform of the given mouse transformation
	
	//Functions dealing with program defaults
	void loadConfig(string sFilename);
	void saveConfig(string sFilename);
	
	obj* objFromXML(string sXMLFilename, Point ptOffset, Point ptVel = Point(0,0));
	Rect getCameraView();		//Return the rectangle, in world position z=0, that the camera can see 
	
	//color.cpp functions
	void updateColors(float32 dt);
	void phaseColor(Color* src, Color dest, float time, bool bPingPong = false);
	void clearColors();
};

void signalHandler(string sSignal); //Stub function for handling signals that come in from our HUD, and passing them on to myEngine
float myAbs(float v);	//Because stinking namespace stuff


#endif
