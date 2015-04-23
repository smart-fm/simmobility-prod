/*
 * Job.cpp
 *
 *  Created on: 23 Apr, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/Job.hpp>

using namespace sim_mob::long_term;

Job::Job(BigSerial id, BigSerial establishmentId, BigSerial sectorId, int timeRestriction,bool isStudent, bool fixedWorkplace):
		 id(id), establishmentId(establishmentId),  sectorId(sectorId), timeRestriction(timeRestriction), isStudent(isStudent), fixedWorkplace(fixedWorkplace){}

Job::~Job() {}

void Job::setId(BigSerial val)
{
	id = val;
}

void Job::setEstablishmentId(BigSerial val)
{
	establishmentId = val;
}

void Job::setSectorId(BigSerial val)
{
	sectorId = val;
}

void Job::setTimeRestriction(int val)
{
	timeRestriction = val;
}

void Job::setIsStudent(bool val)
{
	isStudent = val;
}

void Job::setFixedWorkplace(bool val)
{
	fixedWorkplace = val;
}

BigSerial Job::getId() const
{
	return id;
}

BigSerial Job::getEstablishmentId() const
{
	return establishmentId;
}

BigSerial Job::getSectorId() const
{
	return sectorId;
}

int Job::getTimeRestriction() const
{
	return timeRestriction;
}

bool Job::getIsStudent() const
{
	return isStudent;
}

bool Job::getFixedWorkplace() const
{
	return fixedWorkplace;
}




