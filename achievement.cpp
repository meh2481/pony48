/*
	Pony48 source - achievement.cpp
	Copyright (c) 2014 Mark Hutcheson
*/

#include "Pony48.h"
#include "tinyxml2.h"
#include <float.h>
#include <sstream>
#include <iomanip>

achievement::achievement()
{
	gottenimg = NULL;
	notgottenimg = NULL;
}

void Pony48Engine::loadAchievements()
{
	//Open our achievements file
	string sAchievementFilename = "res/achievements/achievements.xml";
	XMLDocument* doc = new XMLDocument;
	int iErr = doc->LoadFile(sAchievementFilename.c_str());
	if(iErr != XML_NO_ERROR)
	{
		errlog << "Error parsing achievements file: Error " << iErr << endl;
		delete doc;
		return;
	}
	
	//Grab root element
	XMLElement* root = doc->RootElement();
	if(root == NULL)
	{
		errlog << "Error: Root element NULL in XML file. Ignoring..." << endl;
		delete doc;
		return;
	}
	
	//Parse through achievements
	for(XMLElement* ach = root->FirstChildElement("achievement"); ach != NULL; ach = ach->NextSiblingElement("achievement"))
	{
		achievement* a = new achievement;
		const char* cName = ach->Attribute("name");
		if(cName == NULL || !strlen(cName))
		{
			delete a;
			continue;
		}
		
		//Pull in text strings
		const char* cTitle = ach->Attribute("title");
		if(cTitle != NULL && strlen(cTitle))
			a->title = cTitle;
		const char* cGottenTxt = ach->Attribute("gotten");
		if(cGottenTxt != NULL && strlen(cGottenTxt))
			a->gottentxt = cGottenTxt;
		const char* cNotGottenTxt = ach->Attribute("notgotten");
		if(cNotGottenTxt != NULL && strlen(cNotGottenTxt))
			a->notgottentxt = cNotGottenTxt;
		else if(cGottenTxt != NULL && strlen(cGottenTxt))	//Gotten and not-gotten text are the same
			a->notgottentxt = cGottenTxt;
		
		//Pull in images
		const char* cGottenImg = ach->Attribute("gotimg");
		if(cGottenImg != NULL && strlen(cGottenImg))
			a->gottenimg = getImage(cGottenImg);
		const char* cNotGottenImg = ach->Attribute("notgotimg");
		if(cNotGottenImg != NULL && strlen(cNotGottenImg))
			a->notgottenimg = getImage(cNotGottenImg);
		
		//Add to list
		m_achievements[cName] = a;
	}
	delete doc;
}

void Pony48Engine::loadAchievementsGotten(string sAchievements)
{
	for(map<string, achievement*>::iterator i = m_achievements.begin(); i != m_achievements.end(); i++)
	{
		//Just search for achievement names, without care for format. Fine for the achievement names we have here
		if(sAchievements.find(i->first) != string::npos)
			m_achievementsGotten.insert(i->first);
	}
}

string Pony48Engine::saveAchievementsGotten()
{
	//Spit all achievement names into a comma-separated text string
	ostringstream oss;
	for(set<string>::iterator i = m_achievementsGotten.begin(); i != m_achievementsGotten.end(); i++)
		oss << *i << ',';
	return oss.str();
}

void Pony48Engine::achievementGet(string sAch)
{
	if(m_achievementsGotten.count(sAch)) return;	//Make sure this achievement is valid
	if(!m_achievements.count(sAch)) return;			//Make sure we haven't gotten this achievement yet
	
	//Save this achievement
	m_achievementsGotten.insert(sAch);
	
	//Draw this later
	m_achievementsToDraw.push_back(sAch);
	
	//Play getting-achievement sfx
	playSound("bulk_yeah", m_fVoxVolume);
}

void Pony48Engine::cleanupAchievements()
{
	for(map<string, achievement*>::iterator i = m_achievements.begin(); i != m_achievements.end(); i++)
		delete i->second;
	m_achievements.clear();
}

void Pony48Engine::drawAchievementPopup()
{
	//Draw the first achievement on our list
	list<string>::iterator ach = m_achievementsToDraw.begin();
	if(ach != m_achievementsToDraw.end())
	{
		if(m_fStartedShowingAchievement == 0.0f)
			//Start showing achievement
			m_fStartedShowingAchievement = getSeconds();
		else	//We're showing this already, so draw it
		{
			//Move achievement popup vertically to make it appear onscreen
			glPushMatrix();
			string sScene = m_hud->getScene();
			m_hud->setScene("achievementpopup");	//Dupe the HUD into drawing this as well as current scene
			float32 fPos = getSeconds() - m_fStartedShowingAchievement;
			if(fPos < m_fAchievementAppearingTime)	//Achievement appearing onscreen
			{
				float32 fFac = fPos / m_fAchievementAppearingTime;
				glTranslatef(0, (1.0f - fFac) * 3, 0);
			}
			if(fPos > m_fAchievementAppearingTime + m_fShowAchievementTime)	//Achievement disappearing off screen
			{
				fPos -= m_fAchievementAppearingTime + m_fShowAchievementTime;
				float32 fFac = fPos / m_fAchievementVanishingTime;
				glTranslatef(0, fFac * 3, 0);
			}
			//else achievement already onscreen fully, so just draw it
			
			//Set the achievement text and icon properly
			achievement* a = m_achievements[*ach];
			HUDItem* hIt = m_hud->getChild("achname");
			if(hIt != NULL)
			{
				HUDTextbox* txt = (HUDTextbox*)hIt;
				ostringstream oss;
				oss << "Achievement get: \"" << a->title << '\"';
				txt->setText(oss.str());
			}
			hIt = m_hud->getChild("achicon");
			if(hIt != NULL)
			{
				HUDImage* img = (HUDImage*)hIt;
				img->setImage(a->gottenimg);
			}
			
			//Draw the popup HUD
			m_hud->draw(0);
			m_hud->setScene(sScene);
			
			//Done drawing this achievement; pop it off the list
			if(getSeconds() > m_fStartedShowingAchievement + m_fAchievementAppearingTime + m_fShowAchievementTime + m_fAchievementVanishingTime)
			{
				m_fStartedShowingAchievement = 0.0f;
				m_achievementsToDraw.erase(ach);
			}
			glPopMatrix();
		}
	}
}








































