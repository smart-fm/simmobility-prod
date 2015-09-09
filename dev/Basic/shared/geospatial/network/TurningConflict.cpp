//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "TurningConflict.hpp"

using namespace simmobility_network;

TurningConflict::TurningConflict() :
conflictId(0), criticalGap(0), firstConflictDistance(0), firstTurning(NULL), firstTurningId(0), priority(0),
secondConflictDistance(0), secondTurning(NULL), secondTurningId(0), tags(NULL)
{
}

TurningConflict::TurningConflict(const TurningConflict& orig)
{
	this->conflictId = orig.conflictId;
	this->criticalGap = orig.criticalGap;
	this->firstConflictDistance  = orig.firstConflictDistance;
	this->firstTurning = orig.firstTurning;
	this->firstTurningId = orig.firstTurningId;
	this->priority = orig.priority;
	this->secondConflictDistance = orig.secondConflictDistance;
	this->secondTurning = orig.secondTurning;
	this->secondTurningId = orig.secondTurningId;
	this->tags = orig.tags;
	
}

TurningConflict::~TurningConflict()
{
	if(tags)
	{
		delete tags;
		tags = NULL;
	}
}

unsigned int TurningConflict::getConflictId() const
{
	return conflictId;
}

void TurningConflict::setConflictId(unsigned int conflictId)
{
	this->conflictId = conflictId;
}

double TurningConflict::getCriticalGap() const
{
	return criticalGap;
}

void TurningConflict::setCriticalGap(double criticalGap)
{
	this->criticalGap = criticalGap;
}

double TurningConflict::getFirstConflictDistance() const
{
	return firstConflictDistance;
}

void TurningConflict::setFirstConflictDistance(double firstConflictDistance)
{
	this->firstConflictDistance = firstConflictDistance;
}

TurningPath* TurningConflict::getFirstTurning() const
{
	return firstTurning;
}

void TurningConflict::setFirstTurning(TurningPath* firstTurning)
{
	this->firstTurning = firstTurning;
}

unsigned int TurningConflict::getFirstTurningId() const
{
	return firstTurningId;
}

void TurningConflict::setFirstTurningId(unsigned int firstTurningId)
{
	this->firstTurningId = firstTurningId;
}

void TurningConflict::setPriority(unsigned int priority)
{
	this->priority = priority;
}

unsigned int TurningConflict::getPriority() const
{
	return priority;
}

double TurningConflict::getSecondConflictDistance() const
{
	return secondConflictDistance;
}

void TurningConflict::setSecondConflictDistance(double secondConflictDistance)
{
	this->secondConflictDistance = secondConflictDistance;
}

TurningPath* TurningConflict::getSecondTurning() const
{
	return secondTurning;
}

void TurningConflict::setSecondTurning(TurningPath* secondTurning)
{
	this->secondTurning = secondTurning;
}

unsigned int TurningConflict::getSecondTurningId() const
{
	return secondTurningId;
}

void TurningConflict::setSecondTurningId(unsigned int secondTurningId)
{
	this->secondTurningId = secondTurningId;
}

const std::vector<Tag>* TurningConflict::getTags() const
{
	return tags;
}

void TurningConflict::setTags(std::vector<Tag> *tags)
{
	this->tags = tags;
}