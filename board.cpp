/*
    Pony48 source - audio.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "Pony48.h"

void TilePiece::draw()
{
	if(bg!=NULL)
	{
		bg->size = drawSize;
		bg->draw();
	}
	if(seg!=NULL)
	{
		seg->size = drawSize;
		seg->draw();
	}
}

void Pony48Engine::clearBoard()
{
	//Clean up board
	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
		{
			if(m_Board[j][i] != NULL)
			{
				delete m_Board[j][i];
				m_Board[j][i] = NULL;
			}
		}
	}
}

void Pony48Engine::resetBoard()
{
	clearBoard();
	
	//Start with 2 random tiles
	for(int start = 0; start < 2; start++)
	{
		while(true)
		{
			int x = randInt(0, BOARD_WIDTH-1);
			int y = randInt(0, BOARD_HEIGHT-1);
			if(m_Board[x][y] != NULL) continue;
			m_Board[x][y] = loadTile("res/tiles/2.xml");
			break;
		}
	}
	m_iScore = 0;	//Reset score also
}

#define PIECE_MOVE_SPEED 100.0
#define PIECE_APPEAR_SPEED	10.0

void Pony48Engine::updateBoard(float32 dt)
{
	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
		{
			if(m_Board[j][i] == NULL) continue;
			//Check sliding animations
			if(m_Board[j][i]->drawSlide.y < 0)
			{
				m_Board[j][i]->drawSlide.y += dt * PIECE_MOVE_SPEED;
				if(m_Board[j][i]->drawSlide.y > 0)
					m_Board[j][i]->drawSlide.y = 0;
			}
			if(m_Board[j][i]->drawSlide.y > 0)
			{
				m_Board[j][i]->drawSlide.y -= dt * PIECE_MOVE_SPEED;
				if(m_Board[j][i]->drawSlide.y < 0)
					m_Board[j][i]->drawSlide.y = 0;
			}
			if(m_Board[j][i]->drawSlide.x < 0)
			{
				m_Board[j][i]->drawSlide.x += dt * PIECE_MOVE_SPEED;
				if(m_Board[j][i]->drawSlide.x > 0)
					m_Board[j][i]->drawSlide.x = 0;
			}
			if(m_Board[j][i]->drawSlide.x > 0)
			{
				m_Board[j][i]->drawSlide.x -= dt * PIECE_MOVE_SPEED;
				if(m_Board[j][i]->drawSlide.x < 0)
					m_Board[j][i]->drawSlide.x = 0;
			}
			
			//Check appearing animations
			if(m_Board[j][i]->drawSize.x < TILE_WIDTH)
			{
				m_Board[j][i]->drawSize.x += PIECE_APPEAR_SPEED * dt;
				if(m_Board[j][i]->drawSize.x > TILE_WIDTH)
					m_Board[j][i]->drawSize.x = TILE_WIDTH;
			}
			if(m_Board[j][i]->drawSize.y < TILE_HEIGHT)
			{
				m_Board[j][i]->drawSize.y += PIECE_APPEAR_SPEED * dt;
				if(m_Board[j][i]->drawSize.y > TILE_HEIGHT)
					m_Board[j][i]->drawSize.y = TILE_HEIGHT;
			}
		}
	}
}

void Pony48Engine::clearBoardAnimations()
{
	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
		{
			if(m_Board[j][i] == NULL) continue;
			m_Board[j][i]->drawSlide.SetZero();
			m_Board[j][i]->drawSize.Set(TILE_WIDTH, TILE_HEIGHT);
		}
	}
}

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
		}
	}
	
	//Draw tiles themselves (separate loop because alpha issues with animations)
	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
		{
			Point ptDrawPos(-fTotalWidth/2.0 + TILE_SPACING + (TILE_SPACING + TILE_WIDTH) * j,
							fTotalHeight/2.0 - TILE_SPACING - (TILE_SPACING + TILE_HEIGHT) * i);
			//Draw tile
			if(m_Board[j][i] != NULL)
			{
				glPushMatrix();
				glTranslatef(ptDrawPos.x+TILE_WIDTH/2.0+m_Board[j][i]->drawSlide.x, ptDrawPos.y-TILE_HEIGHT/2.0+m_Board[j][i]->drawSlide.y, 0.5);
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
	tmpseg->img = vImages[which];
	tmpseg->size = Point(TILE_WIDTH,TILE_HEIGHT);
	ret->seg = tmpseg;
	
	tmpseg = new physSegment();
	tmpseg->img = getImage("res/gfx/tilebg.png");
	tmpseg->size = Point(TILE_WIDTH,TILE_HEIGHT);
	const char* cBgColor = root->Attribute("bgcolor");
	if(cBgColor != NULL)
		tmpseg->col = colorFromString(cBgColor);
	ret->bg = tmpseg;
	
	delete doc;
	return ret;
}

bool Pony48Engine::movePossible()
{
	return (movePossible(UP) || movePossible(DOWN) || movePossible(LEFT) || movePossible(RIGHT));
}

bool Pony48Engine::movePossible(direction dir)
{
	switch(dir)
	{
		case UP:
			for(int j = 0; j < BOARD_WIDTH; j++)
			{
				for(int i = 1; i < BOARD_HEIGHT; i++)
				{
					if(m_Board[j][i] != NULL)
					{
						if(m_Board[j][i-1] == NULL || m_Board[j][i-1]->value == m_Board[j][i]->value)
							return true;
					}
				}
			}
			break;
		
		case DOWN:
			for(int j = 0; j < BOARD_WIDTH; j++)
			{
				for(int i = BOARD_HEIGHT-2; i >= 0; i--)
				{
					if(m_Board[j][i] != NULL)
					{
						if(m_Board[j][i+1] == NULL || m_Board[j][i+1]->value == m_Board[j][i]->value)
							return true;
					}
				}
			}
			break;
			
		case LEFT:
			for(int i = 0; i < BOARD_HEIGHT; i++)
			{
				for(int j = 1; j < BOARD_WIDTH; j++)
				{
					if(m_Board[j][i] != NULL)
					{
						if(m_Board[j-1][i] == NULL || m_Board[j-1][i]->value == m_Board[j][i]->value)
							return true;
					}
				}
			}
			break;
			
		case RIGHT:
			for(int i = 0; i < BOARD_HEIGHT; i++)
			{
				for(int j = BOARD_WIDTH-2; j >= 0; j--)
				{
					if(m_Board[j][i] != NULL)
					{
						if(m_Board[j+1][i] == NULL || m_Board[j+1][i]->value == m_Board[j][i]->value)
							return true;
					}
				}
			}
			break;
	}
	return false;
}

void Pony48Engine::move(direction dir)
{
	bool moved = false;
	while(slide(dir))
		moved = true;
	moved = join(dir) || moved;
	if(moved)
		placenew();
}

bool Pony48Engine::join(direction dir)
{
	bool mademove = false;
	
	switch(dir)
	{
		case UP:
			for(int j = 0; j < BOARD_WIDTH; j++)
			{
				for(int i = 1; i < BOARD_HEIGHT; i++)
				{
					if(m_Board[j][i] != NULL && m_Board[j][i-1] != NULL)
					{
						if(m_Board[j][i-1]->value == m_Board[j][i]->value)
						{
							addScore(m_Board[j][i]->value * 2);
							delete m_Board[j][i-1];
							ostringstream oss;
							oss << "res/tiles/" << m_Board[j][i]->value * 2 << ".xml";
							m_Board[j][i-1] = loadTile(oss.str());
							m_Board[j][i-1]->drawSize.Set(TILE_WIDTH, TILE_HEIGHT);	//Don't have a newly-created piece make an appear animation here
							delete m_Board[j][i];
							m_Board[j][i] = NULL;
							mademove = true;
						}
					}
				}
			}
			break;
		
		case DOWN:
			for(int j = 0; j < BOARD_WIDTH; j++)
			{
				for(int i = BOARD_HEIGHT-2; i >= 0; i--)
				{
					if(m_Board[j][i] != NULL && m_Board[j][i+1] != NULL)
					{
						if(m_Board[j][i+1]->value == m_Board[j][i]->value)
						{
							addScore(m_Board[j][i]->value * 2);
							delete m_Board[j][i+1];
							ostringstream oss;
							oss << "res/tiles/" << m_Board[j][i]->value * 2 << ".xml";
							m_Board[j][i+1] = loadTile(oss.str());
							m_Board[j][i+1]->drawSize.Set(TILE_WIDTH, TILE_HEIGHT);
							delete m_Board[j][i];
							m_Board[j][i] = NULL;
							mademove = true;
						}
					}
				}
			}
			break;
			
		case LEFT:
			for(int i = 0; i < BOARD_HEIGHT; i++)
			{
				for(int j = 1; j < BOARD_WIDTH; j++)
				{
					if(m_Board[j][i] != NULL && m_Board[j-1][i] != NULL)
					{
						if(m_Board[j-1][i]->value == m_Board[j][i]->value)
						{
							addScore(m_Board[j][i]->value * 2);
							delete m_Board[j-1][i];
							ostringstream oss;
							oss << "res/tiles/" << m_Board[j][i]->value * 2 << ".xml";
							m_Board[j-1][i] = loadTile(oss.str());
							m_Board[j-1][i]->drawSize.Set(TILE_WIDTH, TILE_HEIGHT);
							delete m_Board[j][i];
							m_Board[j][i] = NULL;
							mademove = true;
						}
					}
				}
			}
			break;
			
		case RIGHT:
			for(int i = 0; i < BOARD_HEIGHT; i++)
			{
				for(int j = BOARD_WIDTH-2; j >= 0; j--)
				{
					if(m_Board[j][i] != NULL && m_Board[j+1][i] != NULL)
					{
						if(m_Board[j+1][i]->value == m_Board[j][i]->value)
						{
							addScore(m_Board[j][i]->value * 2);
							delete m_Board[j+1][i];
							ostringstream oss;
							oss << "res/tiles/" << m_Board[j][i]->value * 2 << ".xml";
							m_Board[j+1][i] = loadTile(oss.str());
							m_Board[j+1][i]->drawSize.Set(TILE_WIDTH, TILE_HEIGHT);
							delete m_Board[j][i];
							m_Board[j][i] = NULL;
							mademove = true;
						}
					}
				}
			}
			break;
	}
	return mademove;
}

bool Pony48Engine::slide(direction dir)
{
	bool mademove = false;
	
	switch(dir)
	{
		case UP:
			for(int j = 0; j < BOARD_WIDTH; j++)
			{
				for(int i = 1; i < BOARD_HEIGHT; i++)
				{
					if(m_Board[j][i] != NULL)
					{
						if(m_Board[j][i-1] == NULL)
						{
							m_Board[j][i-1] = m_Board[j][i];
							m_Board[j][i] = NULL;
							mademove = true;
							m_Board[j][i-1]->drawSlide.y -= TILE_HEIGHT + TILE_SPACING;
						}
					}
				}
			}
			break;
		
		case DOWN:
			for(int j = 0; j < BOARD_WIDTH; j++)
			{
				for(int i = BOARD_HEIGHT-2; i >= 0; i--)
				{
					if(m_Board[j][i] != NULL)
					{
						if(m_Board[j][i+1] == NULL)
						{
							m_Board[j][i+1] = m_Board[j][i];
							m_Board[j][i] = NULL;
							mademove = true;
							m_Board[j][i+1]->drawSlide.y += TILE_HEIGHT + TILE_SPACING;
						}
					}
				}
			}
			break;
			
		case LEFT:
			for(int i = 0; i < BOARD_HEIGHT; i++)
			{
				for(int j = 1; j < BOARD_WIDTH; j++)
				{
					if(m_Board[j][i] != NULL)
					{
						if(m_Board[j-1][i] == NULL)
						{
							m_Board[j-1][i] = m_Board[j][i];
							m_Board[j][i] = NULL;
							mademove = true;
							m_Board[j-1][i]->drawSlide.x += TILE_HEIGHT + TILE_SPACING;
						}
					}
				}
			}
			break;
			
		case RIGHT:
			for(int i = 0; i < BOARD_HEIGHT; i++)
			{
				for(int j = BOARD_WIDTH-2; j >= 0; j--)
				{
					if(m_Board[j][i] != NULL)
					{
						if(m_Board[j+1][i] == NULL)
						{
							m_Board[j+1][i] = m_Board[j][i];
							m_Board[j][i] = NULL;
							mademove = true;
							m_Board[j+1][i]->drawSlide.x -= TILE_HEIGHT + TILE_SPACING;
						}
					}
				}
			}
			break;
	}
	return mademove;
}

void Pony48Engine::placenew()
{
	ostringstream oss;
	oss << "res/tiles/" << randInt(1,2) * 2 << ".xml";
	if(movePossible())	//Make sure there aren't no blank spaces or something
	{
		while(true)	//Could possibly hang here for a while
		{
			int x = randInt(0, BOARD_WIDTH-1);
			int y = randInt(0, BOARD_HEIGHT-1);
			if(m_Board[x][y] != NULL) continue;
			m_Board[x][y] = loadTile(oss.str());
			break;
		}
	}
}

void Pony48Engine::addScore(uint32_t amt)
{
	m_iScore += amt;
	if(m_iScore > m_iHighScore)
		m_iHighScore = m_iScore;
	//TODO: Anim stuffs
}



















