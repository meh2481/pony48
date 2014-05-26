/*
	Pony48 source - Pony48.h
	Copyright (c) 2014 Mark Hutcheson
*/
#ifndef PONY48ENGINE_H
#define PONY48ENGINE_H

#include "Engine.h"
#include "bg.h"
#include <vector>
#include <set>

#define DEBUG	//Debug mode; cheat keys and such

#define DEFAULT_WIDTH	800
#define DEFAULT_HEIGHT	600

#define DEFAULT_TIMESCALE	1.0

//const variables
#define BOARD_WIDTH	4
#define BOARD_HEIGHT 4

#define TILE_WIDTH 2.0
#define TILE_HEIGHT 2.0
#define TILE_SPACING 0.25

#define GAMEOVER_KEY_DELAY 0.5

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

class TilePiece
{
public:
	physSegment* seg;
	physSegment* bg;
	int 	value;	//The actual value of the piece (2, 4, 8, etc, or 0 for nothing here)
	
	TilePiece() {seg=NULL;bg=NULL;value=0;};
	~TilePiece() {if(seg!=NULL)delete seg;if(bg!=NULL)delete bg;};
	
	void draw() {if(bg!=NULL)bg->draw();if(seg!=NULL)seg->draw();};
};

typedef enum 
{
	LEFT,
	RIGHT,
	UP,
	DOWN	
} direction;

typedef enum
{
	PLAYING,
	GAMEOVER
} gameMode;

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
	Color m_BoardBg;
	Color m_TileBg;
	Color m_BgCol;
	TilePiece* m_Board[BOARD_WIDTH][BOARD_HEIGHT];
	Vec3 m_BoardRot;
	float32 m_BoardRotAngle;
	uint32_t m_iScore;
	uint32_t m_iHighScore;
	Background* m_bg;
	gameMode m_iCurMode;
	float m_fGameoverKeyDelay;

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

	void hudSignalHandler(string sSignal);	//For handling signals that come from the HUD
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
	
	//audio.cpp functions
	void beatDetect();					//Bounce to da beat
	void loadSongs(string sFilename);	//Load songs to play into memory
	void scrubPause();					//Pauses music with a decreasing-frequency effect
	void scrubResume();					//Resumes music with an increasing-frequency effect
	void soundUpdate(float32 dt);		//Updates audio fx
	
	//board.cpp functions
	void drawBoard();						//Draw the tiles and such on the board
	TilePiece* loadTile(string sFilename);	//Load a tile piece from an XML file
	bool move(direction dir);				//Move in the given direction (if possible)
	bool movePossible(direction dir);		//Test to see if it's possible to move in the given direction
	bool movePossible();					//Test to see if it's possible to move at all
	void placenew();						//Places a new tile at a random location
	void resetBoard();						//Starts a new game
	void clearBoard();						//Clears memory associated with the game board
	void addScore(uint32_t amt);			//Add a value to the score (in function so we can have cool anim stuff)
};

void signalHandler(string sSignal); //Stub function for handling signals that come in from our HUD, and passing them on to the engine
float myAbs(float v);	//Because stinking namespace stuff


#endif
