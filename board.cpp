/*
    Pony48 source - audio.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "Pony48.h"

void Pony48Engine::drawBoard()
{
	float fTotalWidth = BOARD_WIDTH * TILE_WIDTH + (BOARD_WIDTH + 1) * TILE_SPACING;
	float fTotalHeight = BOARD_HEIGHT * TILE_HEIGHT + (BOARD_HEIGHT + 1) * TILE_SPACING;
	//Fill in bg
	fillRect(Point(-fTotalWidth/2.0, fTotalHeight/2.0), Point(fTotalWidth/2.0, -fTotalHeight/2.0), m_BoardBg);
	//Fill in bg for individual tiles
	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
		{
			//Draw tile bg
			Point ptDrawPos(-fTotalWidth/2.0 + TILE_SPACING + (TILE_SPACING + TILE_WIDTH) * j,
							fTotalHeight/2.0 - TILE_SPACING - (TILE_SPACING + TILE_HEIGHT) * i);
			fillRect(ptDrawPos, Point(ptDrawPos.x + TILE_WIDTH, ptDrawPos.y - TILE_HEIGHT), m_TileBg);
			
			//Draw tile
			glPushMatrix();
			glTranslatef(ptDrawPos.x+TILE_WIDTH/2.0, ptDrawPos.y-TILE_HEIGHT/2.0, 0);
			m_Board[j][i].draw();
			glPopMatrix();
		}
	}
}

TilePiece Pony48Engine::loadTile(string sFilename)
{
	TilePiece ret;
	
	
	
	return ret;
}

























