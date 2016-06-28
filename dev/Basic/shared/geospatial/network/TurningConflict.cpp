//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "TurningConflict.hpp"

using namespace sim_mob;

TurningConflict::TurningConflict() :
conflictId(0), criticalGap(0), firstConflictDistance(0), firstTurning(NULL), firstTurningId(0), priority(0),
secondConflictDistance(0), secondTurning(NULL), secondTurningId(0)
{
}

TurningConflict::~TurningConflict()
{
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

const TurningPath* TurningConflict::getFirstTurning() const
{
	return firstTurning;
}

void TurningConflict::setFirstTurning(TurningPath *firstTurning)
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

const TurningPath* TurningConflict::getSecondTurning() const
{
	return secondTurning;
}

void TurningConflict::setSecondTurning(TurningPath *secondTurning)
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
