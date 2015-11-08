//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "TurningPath.hpp"

using namespace sim_mob;

TurningPath::TurningPath() :
turningPathId(0), fromLaneId(0), maxSpeed(0), polyLine(NULL), toLaneId(0), turningGroupId(0),
fromLane(NULL),toLane(NULL)
{
}

TurningPath::~TurningPath()
{
	if(polyLine)
	{
		delete polyLine;
		polyLine = NULL;
	}
	
	//Simply clear the map of conflicts. Conflicts are deleted separately in the 
	//destructor of the road network to avoid double delete (as 2 turning paths share
	//the same pointer to the conflict)
	turningConflicts.clear();
}

unsigned int TurningPath::getTurningPathId() const
{
	return turningPathId;
}

void TurningPath::setTurningPathId(unsigned int turningPathId)
{
	this->turningPathId = turningPathId;
}

const Lane* TurningPath::getFromLane() const
{
	return fromLane;
}

void TurningPath::setFromLane(Lane *fromLane)
{
	this->fromLane = fromLane;
}

unsigned int TurningPath::getFromLaneId() const
{
	return fromLaneId;
}

void TurningPath::setFromLaneId(unsigned int fromLaneId)
{
	this->fromLaneId = fromLaneId;
}

double TurningPath::getMaxSpeed() const
{
	return maxSpeed;
}

void TurningPath::setMaxSpeed(double maxSpeedKmph)
{
	this->maxSpeed = maxSpeedKmph / 3.6;
}

PolyLine* TurningPath::getPolyLine() const
{
	return polyLine;
}

void TurningPath::setPolyLine(PolyLine *polyLine)
{
	this->polyLine = polyLine;
}

const Lane* TurningPath::getToLane() const
{
	return toLane;
}

void TurningPath::setToLane(Lane *toLane)
{
	this->toLane = toLane;
}

unsigned int TurningPath::getToLaneId() const
{
	return toLaneId;
}

void TurningPath::setToLaneId(unsigned int toLaneId)
{
	this->toLaneId = toLaneId;
}

const std::map<const TurningPath *, TurningConflict *>& TurningPath::getTurningConflicts() const
{
	return turningConflicts;
}

unsigned int TurningPath::getTurningGroupId() const
{
	return turningGroupId;
}

void TurningPath::setTurningGroupId(unsigned int turningGroupId)
{
	this->turningGroupId = turningGroupId;
}

void TurningPath::setTurningGroup(TurningGroup* turningGroup)
{
	this->turningGroup = turningGroup;
}

const TurningGroup* TurningPath::getTurningGroup() const
{
	return turningGroup;
}

double TurningPath::getLength() const
{
	return polyLine->getLength();
}

double TurningPath::getWidth() const
{
	return (fromLane->getWidth() + toLane->getWidth()) / 2;
}

void TurningPath::addTurningConflict(const TurningPath *other, TurningConflict *conflict)
{
	turningConflicts.insert(std::make_pair(other, conflict));
}

const TurningConflict* TurningPath::getTurningConflict(const TurningPath *turningPath) const
{
	//Get the conflict on this turning shared with the given turning
	std::map<const TurningPath *, TurningConflict *>::const_iterator itConflicts = turningConflicts.find(turningPath);
	
	if(itConflicts != turningConflicts.end())
	{
		return itConflicts->second;
	}
	else
	{
		return NULL;
	}
}