//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "TurningConflict.hpp"

using namespace simmobility_network;

TurningConflict::TurningConflict(unsigned int id, double first_cd, unsigned int firstTurningId, double second_cd,
								 unsigned int secondTurningId, Tag *tag) :
conflictId(id), firstConflictDistance(first_cd), firstTurningId(firstTurningId), secondConflictDistance(second_cd),
secondTurningId(secondTurningId), tag(tag)
{
}

TurningConflict::TurningConflict(const TurningConflict& orig)
{
	this->conflictId = orig.conflictId;
	this->firstConflictDistance  = orig.firstConflictDistance;
	this->firstTurningId = orig.firstTurningId;
	this->secondConflictDistance = orig.secondConflictDistance;
	this->secondTurningId = orig.secondTurningId;
	this->tag = orig.tag;
}

TurningConflict::~TurningConflict()
{
	if(tag)
	{
		delete tag;
		tag = NULL;
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

double TurningConflict::getFirstConflictDistance() const
{
	return firstConflictDistance;
}

void TurningConflict::setFirstConflictDistance(double firstConflictDistance)
{
	this->firstConflictDistance = firstConflictDistance;
}

unsigned int TurningConflict::getFirstTurningId() const
{
	return firstTurningId;
}

void TurningConflict::setFirstTurningId(unsigned int firstTurningId)
{
	this->firstTurningId = firstTurningId;
}

double TurningConflict::getSecondConflictDistance() const
{
	return secondConflictDistance;
}

void TurningConflict::setSecondConflictDistance(double secondConflictDistance)
{
	this->secondConflictDistance = secondConflictDistance;
}

unsigned int TurningConflict::getSecondTurningId() const
{
	return secondTurningId;
}

void TurningConflict::setSecondTurningId(unsigned int secondTurningId)
{
	this->secondTurningId = secondTurningId;
}

Tag* TurningConflict::getTag() const
{
	return tag;
}

void TurningConflict::setTag(Tag* tag)
{
	this->tag = tag;
}
