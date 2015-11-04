//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "TurningGroup.hpp"
#include "util/GeomHelpers.hpp"

using namespace sim_mob;

TurningGroup::TurningGroup() :
turningGroupId(0), fromLinkId(0), nodeId(0), phases(""), groupRule(TURNING_GROUP_RULE_NO_STOP_SIGN), toLinkId(0), visibility(0),
numTurningPaths(0), length(0)
{
}

TurningGroup::~TurningGroup()
{
	//Delete the turning paths

	//Iterate through the map and delete the turnings
	std::map<unsigned int, std::map<unsigned int, TurningPath *> >::iterator itTurnings = turningPaths.begin();
	while (itTurnings != turningPaths.end())
	{
		clear_delete_map(itTurnings->second);
		++itTurnings;
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

TurningGroupRule TurningGroup::getRule() const
{
	return groupRule;
}

void TurningGroup::setRule(TurningGroupRule rules)
{
	this->groupRule = rules;
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

const std::map<unsigned int, std::map<unsigned int, TurningPath *> >& TurningGroup::getTurningPaths() const
{
	return turningPaths;
}

unsigned int TurningGroup::getNumTurningPaths() const
{
	return numTurningPaths;
}

double TurningGroup::getLength() const
{
	return length;
}

void TurningGroup::addTurningPath(TurningPath *turningPath)
{
	//Update the turning group length
	const PolyPoint from = turningPath->getFromLane()->getPolyLine()->getLastPoint();
	const PolyPoint to = turningPath->getToLane()->getPolyLine()->getLastPoint();
	
	double distance = dist(from.getX(), from.getY(), to.getX(), to.getY());
	length = ((length * numTurningPaths) + distance) / (numTurningPaths + 1);
	
	//Look up the entry for turning path with the same from lane
	std::map<unsigned int, std::map<unsigned int, TurningPath *> >::iterator itPaths = turningPaths.find(turningPath->getFromLaneId());
	
	if(itPaths != turningPaths.end())
	{
		//Update the inner map, as the entry for this "from lane" exists
		itPaths->second.insert(std::make_pair(turningPath->getToLaneId(), turningPath));
	}
	else
	{
		//Create a new inner map as it doesn't exist for this "from lane"
		std::map<unsigned int, TurningPath *> innerMap;
		innerMap.insert(std::make_pair(turningPath->getToLaneId(), turningPath));
		
		//Add a new entry
		turningPaths.insert(std::make_pair(turningPath->getFromLaneId(), innerMap));
	}
	
	//Increment number of turning paths
	numTurningPaths++;
}

const std::map<unsigned int, TurningPath *>* TurningGroup::getTurningPaths(unsigned int fromLaneId) const
{
	std::map<unsigned int, std::map<unsigned int, TurningPath *> >::const_iterator itPathsFrom = turningPaths.find(fromLaneId);
	
	if(itPathsFrom != turningPaths.end())
	{
		return &(itPathsFrom->second);
	}
	else
	{
		return NULL;
	}
}
