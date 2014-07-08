/*
    Pony48 source - audio.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "Pony48.h"

TilePiece::TilePiece()
{
	seg=NULL;
	bg=NULL;
	value=0;
	drawSlide.SetZero();
	drawSize.SetZero();
	destx=desty=-1;
	iAnimDir=1;
	joined=false;
}

TilePiece::~TilePiece()
{
	if(seg!=NULL)
		delete seg;
	if(bg!=NULL)
		delete bg;
}

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
	//Clean up animations
	for(list<TilePiece*>::iterator i = m_lSlideJoinAnimations.begin(); i != m_lSlideJoinAnimations.end(); i++)
		delete *i;
	m_lSlideJoinAnimations.clear();
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
	m_highestTile = NULL;
	m_fLastMovedSec = getSeconds();
	m_bHasBoredVox = false;
}

//Update slide-and-join animations if the to-join-to piece slid
void Pony48Engine::pieceSlid(int startx, int starty, int endx, int endy)
{
	for(list<TilePiece*>::iterator i = m_lSlideJoinAnimations.begin(); i != m_lSlideJoinAnimations.end(); i++)
	{
		if((*i)->destx == startx && (*i)->desty == starty)
		{
			(*i)->drawSlide.x += (startx - endx) * (TILE_WIDTH + TILE_SPACING);
			(*i)->drawSlide.y -= (starty - endy) * (TILE_HEIGHT + TILE_SPACING);
			(*i)->destx = endx;
			(*i)->desty = endy;
		}
	}
}

#define PIECE_MOVE_SPEED 60.0
#define PIECE_APPEAR_SPEED	8.0
#define PIECE_BOUNCE_SPEED	4.5
#define PIECE_BOUNCE_SIZE TILE_WIDTH+TILE_SPACING*2.2
#define ARROW_SPEED 	3.5
#define ARROW_RESET		(TILE_WIDTH + TILE_SPACING)
#define SOUND_DIV_FAC	2.5f

void Pony48Engine::updateBoard(float32 dt)
{
	m_fArrowAdd += dt * ARROW_SPEED;
	if(m_fArrowAdd >= ARROW_RESET)
		m_fArrowAdd -= ARROW_RESET;
	//Check slide-and-join animations
	for(list<TilePiece*>::iterator i = m_lSlideJoinAnimations.begin(); i != m_lSlideJoinAnimations.end();)
	{
		if((*i)->drawSlide.y < 0)
		{
			(*i)->drawSlide.y += dt * PIECE_MOVE_SPEED;
			if((*i)->drawSlide.y > 0)
				(*i)->drawSlide.y = 0;
		}
		else if((*i)->drawSlide.y > 0)
		{
			(*i)->drawSlide.y -= dt * PIECE_MOVE_SPEED;
			if((*i)->drawSlide.y < 0)
				(*i)->drawSlide.y = 0;
		}
		
		if((*i)->drawSlide.x < 0)
		{
			(*i)->drawSlide.x += dt * PIECE_MOVE_SPEED;
			if((*i)->drawSlide.x > 0)
				(*i)->drawSlide.x = 0;
		}
		else if((*i)->drawSlide.x > 0)
		{
			(*i)->drawSlide.x -= dt * PIECE_MOVE_SPEED;
			if((*i)->drawSlide.x < 0)
				(*i)->drawSlide.x = 0;
		}
		
		if((*i)->drawSlide.x == 0 && (*i)->drawSlide.y == 0)
		{
			if((*i)->destx >= 0 && (*i)->desty >= 0)
			{
				//Hit the end; join with destination tile
				if(m_Board[(*i)->destx][(*i)->desty] != NULL)
				{
					addScore((*i)->value * 2);
					float32 xPos = ((float32)(*i)->destx - 2.0f);
					if(xPos >= 0) xPos++;
					xPos /= SOUND_DIV_FAC;
					playSound("jointile", m_fSoundVolume, xPos);
					if(m_highestTile == m_Board[(*i)->destx][(*i)->desty]) 
						m_highestTile = NULL;
					rumbleController(0.2, 0.1);
					delete m_Board[(*i)->destx][(*i)->desty];
					ostringstream oss;
					oss << "res/tiles/" << min((*i)->value * 2, MAX_TILE_VALUE) << ".xml";	//"Duh, muffins" is highest possible tile
					m_Board[(*i)->destx][(*i)->desty] = loadTile(oss.str());
					m_Board[(*i)->destx][(*i)->desty]->drawSize.Set(TILE_WIDTH+0.001, TILE_HEIGHT+0.001);	//Start bounce animation
					m_Board[(*i)->destx][(*i)->desty]->iAnimDir = 1;
					if(!(m_highestTile) || m_highestTile->value < m_Board[(*i)->destx][(*i)->desty]->value)
					{
						m_highestTile = m_Board[(*i)->destx][(*i)->desty];
						m_newHighTile->firing = true;
					}
				}
				else
					errlog << "Err board[x][y] == NULL" << (*i)->destx << "," << (*i)->desty << endl;
			}
			else
				errlog << "Err destx/y < 0: " << (*i)->destx << "," << (*i)->desty << endl;
			delete (*i);
			i = m_lSlideJoinAnimations.erase(i);
			continue;
		}
		i++;
	}
	
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
			else if(m_Board[j][i]->drawSize.x > TILE_WIDTH)	//And bouncing animations
			{
				m_Board[j][i]->drawSize.x += m_Board[j][i]->iAnimDir * PIECE_BOUNCE_SPEED * dt;
				if(m_Board[j][i]->drawSize.x > PIECE_BOUNCE_SIZE)
					m_Board[j][i]->iAnimDir = -1;	//Reverse animation direction
				else if(m_Board[j][i]->drawSize.x < TILE_WIDTH)
					m_Board[j][i]->drawSize.x = TILE_WIDTH;	//This also stops bouncing
			}
			if(m_Board[j][i]->drawSize.y < TILE_HEIGHT)
			{
				m_Board[j][i]->drawSize.y += PIECE_APPEAR_SPEED * dt;
				if(m_Board[j][i]->drawSize.y > TILE_HEIGHT)
					m_Board[j][i]->drawSize.y = TILE_HEIGHT;
			}
			else if(m_Board[j][i]->drawSize.y > TILE_HEIGHT)
			{
				m_Board[j][i]->drawSize.y += m_Board[j][i]->iAnimDir * PIECE_BOUNCE_SPEED * dt;
				if(m_Board[j][i]->drawSize.y > PIECE_BOUNCE_SIZE)
					m_Board[j][i]->iAnimDir = -1;
				else if(m_Board[j][i]->drawSize.y < TILE_HEIGHT)
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
	
	//Check slide-and-join anims
	for(list<TilePiece*>::iterator i = m_lSlideJoinAnimations.begin(); i != m_lSlideJoinAnimations.end();)
	{
		//Hit the end; join with destination tile
		if((*i)->destx >= 0 && (*i)->desty >= 0)
		{
			if(m_Board[(*i)->destx][(*i)->desty] != NULL)
			{
				addScore((*i)->value * 2);
				float32 xPos = ((float32)(*i)->destx - 2.0f);
				if(xPos >= 0) xPos++;
				xPos /= SOUND_DIV_FAC;
				playSound("jointile", m_fSoundVolume, xPos);
				if(m_highestTile == m_Board[(*i)->destx][(*i)->desty]) 
					m_highestTile = NULL;
				rumbleController(0.2, 0.1);
				delete m_Board[(*i)->destx][(*i)->desty];
				ostringstream oss;
				oss << "res/tiles/" << min((*i)->value * 2, MAX_TILE_VALUE) << ".xml";	//"Duh, muffins" is highest possible tile
				m_Board[(*i)->destx][(*i)->desty] = loadTile(oss.str());
				m_Board[(*i)->destx][(*i)->desty]->drawSize.Set(TILE_WIDTH, TILE_HEIGHT);	//Don't have a newly-created piece make an appear animation here
				if(!(m_highestTile) || m_highestTile->value < m_Board[(*i)->destx][(*i)->desty]->value)
				{
					m_highestTile = m_Board[(*i)->destx][(*i)->desty];
					m_newHighTile->firing = true;
				}
			}
		}
		//Wipe this out (Step through list this way, rather than incrementing i)
		delete (*i);
		i = m_lSlideJoinAnimations.erase(i);
	}
}

#define TILEBG_DRAWZ 	0.3
#define JOINANIM_DRAWZ 	0.5
#define TILE_DRAWZ		0.7
#define MOVEARROW_DRAWZ	0.9
#define MOVEARROW_FADEOUTDIST	(TILE_WIDTH * 0.5f)
#define MOVEARROW_FADEINDIST	(TILE_WIDTH * 0.95f)
void Pony48Engine::drawBoard()
{
	float fTotalWidth = BOARD_WIDTH * TILE_WIDTH + (BOARD_WIDTH + 1) * TILE_SPACING;
	float fTotalHeight = BOARD_HEIGHT * TILE_HEIGHT + (BOARD_HEIGHT + 1) * TILE_SPACING;
	//Fill in bg
	fillRect(Point(-fTotalWidth/2.0, fTotalHeight/2.0), Point(fTotalWidth/2.0, -fTotalHeight/2.0), m_BoardBg);	//Draw at z = 0
	//Fill in bg for individual tiles
	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
		{
			//Draw tile bg
			glPushMatrix();
			glTranslatef(0, 0, TILEBG_DRAWZ);
			Point ptDrawPos(-fTotalWidth/2.0 + TILE_SPACING + (TILE_SPACING + TILE_WIDTH) * j,
							fTotalHeight/2.0 - TILE_SPACING - (TILE_SPACING + TILE_HEIGHT) * i);
			fillRect(ptDrawPos, Point(ptDrawPos.x + TILE_WIDTH, ptDrawPos.y - TILE_HEIGHT), m_TileBg[j][i]);
			glPopMatrix();
		}
	}
	
	//Draw joining-tile animations
	for(list<TilePiece*>::iterator i = m_lSlideJoinAnimations.begin(); i != m_lSlideJoinAnimations.end(); i++)
	{
		Point ptDrawPos(-fTotalWidth/2.0 + TILE_SPACING + (TILE_SPACING + TILE_WIDTH) * (*i)->destx,
						fTotalHeight/2.0 - TILE_SPACING - (TILE_SPACING + TILE_HEIGHT) * (*i)->desty);
		glPushMatrix();
		glTranslatef(ptDrawPos.x+TILE_WIDTH/2.0+(*i)->drawSlide.x, ptDrawPos.y-TILE_HEIGHT/2.0+(*i)->drawSlide.y, JOINANIM_DRAWZ);
		(*i)->draw();
		glPopMatrix();
	}
	
	//Draw tiles themselves (separate loop because z-order alpha issues with animations)
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
				glTranslatef(ptDrawPos.x+TILE_WIDTH/2.0+m_Board[j][i]->drawSlide.x, ptDrawPos.y-TILE_HEIGHT/2.0+m_Board[j][i]->drawSlide.y, TILE_DRAWZ);
				m_Board[j][i]->draw();
				glPopMatrix();
			}
		}
	}
	
	//Draw particle fx for highest tile
	if(m_highestTile != NULL)
	{
		for(int i = 0; i < BOARD_HEIGHT; i++)
		{
			for(int j = 0; j < BOARD_WIDTH; j++)
			{
				//Draw tile
				if(m_Board[j][i] == m_highestTile)
				{
					Point ptDrawPos(-fTotalWidth/2.0 + TILE_SPACING + (TILE_SPACING + TILE_WIDTH) * j,
								fTotalHeight/2.0 - TILE_SPACING - (TILE_SPACING + TILE_HEIGHT) * i);
					glPushMatrix();
					glTranslatef(ptDrawPos.x+TILE_WIDTH/2.0+m_Board[j][i]->drawSlide.x, ptDrawPos.y-TILE_HEIGHT/2.0+m_Board[j][i]->drawSlide.y, TILE_DRAWZ + 0.1);
					m_newHighTile->img = m_highestTile->bg->img;
					m_newHighTile->draw();
					m_newHighTile->img = m_highestTile->seg->img;
					m_newHighTile->draw();
					glPopMatrix();
					break;
				}
			}
		}
		
	}
	
	//Draw arrows for direction the mouse will move the board
	if(m_iMouseControl >= MOUSE_MOVE_TRIP_AMT && m_iCurMode == PLAYING)
	{
		glPushMatrix();
		Point ptMoveDir = worldPosFromCursor(getCursorPos());
		//Rotate first to simplify logic. Hooray!
		switch(getDirOfVec2(ptMoveDir))
		{
			case UP:
				glRotatef(90, 0, 0, 1);
				break;
				
			case DOWN:
				glRotatef(-90, 0, 0, 1);
				break;
				
			case LEFT:
				glRotatef(180, 0, 0, 1);
				break;
		}
		//Determine the drawing alpha based on how far away from the center the mouse is
		float32 fDestAlpha = min(fabs(ptMoveDir.Length() / (getCameraView().height() / 2.0)) - 0.4, 0.4);
		//Draw 16 arrows pointing in the direction we'll move
		for(int y = 0; y < BOARD_HEIGHT; y++)
		{
			for(int x = 0; x < BOARD_WIDTH; x++)
			{
				//Position to draw this arrow at
				Point ptDrawPos(-fTotalWidth/2.0 + (TILE_SPACING + TILE_WIDTH) * x + TILE_WIDTH / 2.0 + TILE_SPACING + m_fArrowAdd,
								fTotalHeight/2.0 - (TILE_SPACING + TILE_WIDTH) * y - TILE_HEIGHT / 2.0 - TILE_SPACING);
				
				//If this arrow is reaching the end of its lifespan, fade out
				float32 fDrawAlpha = fDestAlpha;
				if(m_fArrowAdd >= MOVEARROW_FADEOUTDIST && x == BOARD_WIDTH - 1)
					fDrawAlpha *= 1.0f - ((m_fArrowAdd - MOVEARROW_FADEOUTDIST) / MOVEARROW_FADEOUTDIST);
				fDrawAlpha = min(fDrawAlpha, 1.0f);
				fDrawAlpha = max(fDrawAlpha, 0.0f);
				
				//Now draw
				glColor4f(1,1,1,fDrawAlpha);
				glPushMatrix();
				glTranslatef(ptDrawPos.x, ptDrawPos.y, MOVEARROW_DRAWZ);
				m_imgMouseMoveArrow->render(Point(1,1));
				glPopMatrix();
				
				//See if we should draw new arrow spawning
				if(!x && (ARROW_RESET - m_fArrowAdd) <= MOVEARROW_FADEINDIST)
				{
					fDrawAlpha = fDestAlpha;
					fDrawAlpha *= 1.0 - (ARROW_RESET - m_fArrowAdd) / MOVEARROW_FADEINDIST;
					fDrawAlpha = min(fDrawAlpha, 1.0f);
					fDrawAlpha = max(fDrawAlpha, 0.0f);
					glColor4f(1,1,1,fDrawAlpha);
					glPushMatrix();
					glTranslatef(ptDrawPos.x - (TILE_SPACING + TILE_WIDTH), ptDrawPos.y, MOVEARROW_DRAWZ);
					m_imgMouseMoveArrow->render(Point(1,1));
					glPopMatrix();
				}
			}
		}
		glPopMatrix();
	}
}

direction Pony48Engine::getDirOfVec2(Point ptVec)
{
	float32 fAngle = atan2(ptVec.y, ptVec.x);
	if(fAngle > PI/4.0 && fAngle < 3.0*PI/4.0)	//Up
		return UP;
	else if(fAngle > -3.0*PI/4.0 && fAngle < -PI/4.0)	//Down
		return DOWN;
	else if(fAngle < PI/4.0 && fAngle > -PI/4.0)	//Right
		return RIGHT;
	else	//Left
		return LEFT;
	
	//Unreachable
	return UP;
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
	
	for(XMLElement* sound = root->FirstChildElement("sound"); sound != NULL; sound = sound->NextSiblingElement("sound"))
	{
		bool bPlaySoundImmediately = true;
		const char* cType = sound->Attribute("type");
		if(cType)
		{
			string sType = cType;
			if(sType == "newhigh")
				//I have no idea how this happens, but apparently the highest tile can be this before it even returns. Wat
				bPlaySoundImmediately = ((m_highestTile == NULL) || (m_highestTile->value < ret->value));
		}
		
		//Save all sfx if there's multiple
		vector<string> vSounds;
		
		for(XMLElement* fx = sound->FirstChildElement("fx"); fx != NULL; fx = fx->NextSiblingElement("fx"))
		{
			const char* cPath = fx->Attribute("path");
			const char* cName = fx->Attribute("name");
			if(cPath && cName)
			{
				createSound(cPath, cName);
				vSounds.push_back(cName);
			}
		}
		//Play one of these randomly
		if(vSounds.size() && bPlaySoundImmediately)
			playSound(vSounds[randInt(0, vSounds.size() - 1)], m_fVoxVolume);
	}
	
	physSegment* tmpseg = new physSegment();
	int which = randInt(0, vImages.size()-1);
	tmpseg->img = vImages[which];
	tmpseg->size = Point(TILE_WIDTH,TILE_HEIGHT);
	ret->seg = tmpseg;
	
	tmpseg = new physSegment();
	tmpseg->img = getImage("res/tiles/tilebg.png");
	tmpseg->size = Point(TILE_WIDTH,TILE_HEIGHT);
	const char* cBgColor = root->Attribute("bgcolor");
	if(cBgColor != NULL)
		tmpseg->col = colorFromString(cBgColor);
	ret->bg = tmpseg;
	ret->origCol = tmpseg->col;
	
	delete doc;
	return ret;
}

bool Pony48Engine::movePossible()
{
	//Have to take animations into account here, otherwise we could gameover when moves are still possible
	return (m_lSlideJoinAnimations.size() || movePossible(UP) || movePossible(DOWN) || movePossible(LEFT) || movePossible(RIGHT));
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
	m_fLastMovedSec = getSeconds();
	m_bHasBoredVox = false;
	clearBoardAnimations();	//Wipe out any movement animations that are still playing
	bool moved = false;
	while(slide(dir))	//Slide as far as we can
		moved = true;
	moved = join(dir) || moved;	//Join once
	while(slide(dir))	//Slide again!
		moved = true;
	if(moved)
		placenew();	//Create a new tile if we've successfully moved
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
							m_Board[j][i]->destx = j;
							m_Board[j][i]->desty = i-1;
							m_Board[j][i]->drawSlide.y -= TILE_HEIGHT + TILE_SPACING;
							m_lSlideJoinAnimations.push_back(m_Board[j][i]);
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
							m_Board[j][i]->destx = j;
							m_Board[j][i]->desty = i+1;
							m_Board[j][i]->drawSlide.y += TILE_HEIGHT + TILE_SPACING;
							m_lSlideJoinAnimations.push_back(m_Board[j][i]);
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
							m_Board[j][i]->destx = j-1;
							m_Board[j][i]->desty = i;
							m_Board[j][i]->drawSlide.x += TILE_WIDTH + TILE_SPACING;
							m_lSlideJoinAnimations.push_back(m_Board[j][i]);
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
							m_Board[j][i]->destx = j+1;
							m_Board[j][i]->desty = i;
							m_Board[j][i]->drawSlide.x -= TILE_WIDTH + TILE_SPACING;
							m_lSlideJoinAnimations.push_back(m_Board[j][i]);
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
							pieceSlid(j, i, j, i-1);
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
							pieceSlid(j, i, j, i+1);
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
							m_Board[j-1][i]->drawSlide.x += TILE_WIDTH + TILE_SPACING;
							pieceSlid(j, i, j-1, i);
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
							m_Board[j+1][i]->drawSlide.x -= TILE_WIDTH + TILE_SPACING;
							pieceSlid(j, i, j+1, i);
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

void Pony48Engine::spawnScoreParticles(uint32_t amt)
{
	ostringstream oss;
	oss << "res/particles/" << amt << ".xml";
	if(m_ScoreParticles.count(oss.str()))
	{
		m_ScoreParticles[oss.str()]->firing = true;
	}
	else if(ttvfs::FileExists(oss.str().c_str()))
	{
		ParticleSystem* pSys = new ParticleSystem();
		pSys->fromXML(oss.str());
		pSys->init();
		pSys->firing = true;
		m_ScoreParticles[oss.str()] = pSys;
	}
}

void Pony48Engine::addScore(uint32_t amt)
{
	m_iScore += amt;
	if(m_iScore > m_iHighScore)
		m_iHighScore = m_iScore;
	spawnScoreParticles(amt);
}



















