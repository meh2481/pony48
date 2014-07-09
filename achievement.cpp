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
	
	for(XMLElement* ach = root->FirstChildElement("achievement"); ach != NULL; ach = ach->NextSiblingElement("achievement"))
	{
		achievement* a = new achievement;
		const char* cName = ach->Attribute("name");
		if(cName == NULL || !strlen(cName))
		{
			delete a;
			continue;
		}
		
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
		
		const char* cGottenImg = ach->Attribute("gotimg");
		if(cGottenImg != NULL && strlen(cGottenImg))
			a->gottenimg = getImage(cGottenImg);
		const char* cNotGottenImg = ach->Attribute("notgotimg");
		if(cNotGottenImg != NULL && strlen(cNotGottenImg))
			a->notgottenimg = getImage(cNotGottenImg);
		
		m_achievements[cName] = a;
		
	}
	delete doc;
}

void Pony48Engine::loadAchievementsGotten(string sAchievements)
{
	for(map<string, achievement*>::iterator i = m_achievements.begin(); i != m_achievements.end(); i++)
	{
		if(sAchievements.find(i->first) != string::npos)
			m_achievementsGotten.insert(i->first);
	}
}

string Pony48Engine::saveAchievementsGotten()
{
	ostringstream oss;
	for(set<string>::iterator i = m_achievementsGotten.begin(); i != m_achievementsGotten.end(); i++)
		oss << *i << ',';
	return oss.str();
}

void Pony48Engine::achievementGet(string sAch)
{
	if(m_achievementsGotten.count(sAch)) return;
	if(!m_achievements.count(sAch)) return;
	m_achievementsGotten.insert(sAch);
	
	//TODO Fanfare
}

void Pony48Engine::cleanupAchievements()
{
	for(map<string, achievement*>::iterator i = m_achievements.begin(); i != m_achievements.end(); i++)
		delete i->second;
	m_achievements.clear();
}

void Pony48Engine::drawAchievementPopup()
{
	
}








































