//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "TurningGroup.hpp"

using namespace sim_mob;

TurningGroup::TurningGroup() :
turningGroupId(0), fromLinkId(0), nodeId(0), phases(""), rules(TURNING_GROUP_RULE_NO_STOP_SIGN), toLinkId(0), visibility(0)
{
}

TurningGroup::~TurningGroup()
{
	//Delete the turning paths

	//Iterate through the outer map
	std::map<unsigned int, std::map<unsigned int, TurningPath *> >::iterator itOuterMap = turningPaths.begin();
	while (itOuterMap != turningPaths.end())
	{
		//Iterate through the inner map
		std::map<unsigned int, TurningPath *>::iterator itInnerMap = itOuterMap->second.begin();
		while (itInnerMap != itOuterMap->second.end())
		{
			//Delete the turning group
			delete itInnerMap->second;
			itInnerMap->second = NULL;
			++itInnerMap;
		}

		//Clear the inner map
		itOuterMap->second.clear();

		++itOuterMap;
	}

	//Clear the map
	turningPaths.clear();
}

unsigned int TurningGroup::getTurningGroupId() const
{
	return turningGroupId;
}

void TurningGroup::setTurningGroupId(unsigned int turningGroupId)
{
	this->turningGroupId = turningGroupId;
}

unsigned int TurningGroup::getFromLinkId() const
{
	return fromLinkId;
}

void TurningGroup::setFromLinkId(unsigned int fromLinkId)
{
	this->fromLinkId = fromLinkId;
}

unsigned int TurningGroup::getNodeId() const
{
	return nodeId;
}

void TurningGroup::setNodeId(unsigned int nodeId)
{
	this->nodeId = nodeId;
}

std::string TurningGroup::getPhases() const
{
	return phases;
}

void TurningGroup::setPhases(std::string phases)
{
	this->phases = phases;
}

TurningGroupRules TurningGroup::getRules() const
{
	return rules;
}

void TurningGroup::setRules(TurningGroupRules rules)
{
	this->rules = rules;
}

unsigned int TurningGroup::getToLinkId() const
{
	return toLinkId;
}

void TurningGroup::setToLinkId(unsigned int toLinkId)
{
	this->toLinkId = toLinkId;
}

double TurningGroup::getVisibility() const
{
	return visibility;
}

void TurningGroup::setVisibility(double visibility)
{
	this->visibility = visibility;
}

void TurningGroup::addTurningPath(TurningPath* turningPath)
{
	//Find the map entry having the key given by the 'from lane' of the turning path
	std::map<unsigned int, std::map<unsigned int, TurningPath *> >::iterator itOuter = turningPaths.find(turningPath->getFromLaneId());

	//Check if such an entry exists
	if (itOuter != turningPaths.end())
	{
		//Entry found, so just add a new entry in the inner map using the 'to lane' as the key
		itOuter->second.insert(std::make_pair(turningPath->getToLaneId(), turningPath));
	}
	//Inner map doesn't exist
	else
	{
		//Create the inner map
		std::map<unsigned int, TurningPath *> innerMap;

		//Make a new entry in the inner map
		innerMap.insert(std::make_pair(turningPath->getToLaneId(), turningPath));

		//Make a new entry into the outer map
		turningPaths.insert(std::make_pair(turningPath->getFromLaneId(), innerMap));
	}
}