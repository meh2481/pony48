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
			glPushMatrix();
			glTranslatef(0, 0, 0.2);
			Point ptDrawPos(-fTotalWidth/2.0 + TILE_SPACING + (TILE_SPACING + TILE_WIDTH) * j,
							fTotalHeight/2.0 - TILE_SPACING - (TILE_SPACING + TILE_HEIGHT) * i);
			fillRect(ptDrawPos, Point(ptDrawPos.x + TILE_WIDTH, ptDrawPos.y - TILE_HEIGHT), m_TileBg);
			glPopMatrix();
			
			//Draw tile
			if(m_Board[j][i] != NULL)
			{
				glPushMatrix();
				glTranslatef(ptDrawPos.x+TILE_WIDTH/2.0, ptDrawPos.y-TILE_HEIGHT/2.0, 0.5);
				m_Board[j][i]->draw();
				glPopMatrix();
			}
		}
	}
}

TilePiece* Pony48Engine::loadTile(string sFilename)
{
	TilePiece* ret = new TilePiece();
	XMLDocument* doc = new XMLDocument();
    int iErr = doc->LoadFile(sFilename.c_str());
	if(iErr != XML_NO_ERROR)
	{
		errlog << "Error parsing XML file " << sFilename << ": Error " << iErr << endl;
		delete doc;
		return NULL;
	}

    XMLElement* root = doc->FirstChildElement("tile");
    if(root == NULL)
	{
		errlog << "Error: No toplevel \"tile\" item in XML file " << sFilename << endl;
		delete doc;
		return NULL;
	}
	root->QueryIntAttribute("value", &ret->value);
	vector<Image*> vImages;
	for(XMLElement* img = root->FirstChildElement("img"); img != NULL; img = img->NextSiblingElement("img"))
	{
		const char* cPath = img->Attribute("path");
		if(cPath != NULL)
			vImages.push_back(getImage(cPath));
	}
	
	physSegment* tmpseg = new physSegment();
	int which = randInt(0, vImages.size()-1);
	errlog << "Which: " << which << endl;
	tmpseg->img = vImages[which];
	tmpseg->size = Point(TILE_WIDTH,TILE_HEIGHT);
	ret->seg = tmpseg;
	
	tmpseg = new physSegment();
	tmpseg->img = getImage("res/gfx/tilebg.png");
	tmpseg->size = Point(TILE_WIDTH,TILE_HEIGHT);
	ret->bg = tmpseg;
	
	delete doc;
	return ret;
}

void Pony48Engine::move(direction dir)
{
	switch(dir)
	{
		case UP:
			break;
		
		case DOWN:
			break;
			
		case LEFT:
			break;
			
		case RIGHT:
			break;
	}
}























