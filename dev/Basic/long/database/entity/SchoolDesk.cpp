/*
 * SchoolDesk.cpp
 *
 *  Created on: 26 Apr 2018
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

#include "SchoolDesk.hpp"

using namespace sim_mob::long_term;

SchoolDesk::SchoolDesk(BigSerial schoolDeskId, BigSerial schoolId) : schoolDeskId(schoolDeskId),schoolId(schoolId){}

SchoolDesk::~SchoolDesk(){}

SchoolDesk::SchoolDesk( const SchoolDesk &source)
{
	this->schoolId = source.schoolId;
	this->schoolDeskId = source.schoolDeskId;
}

SchoolDesk& SchoolDesk::operator=(const SchoolDesk& source)
{
	this->schoolId = source.schoolId;
	this->schoolDeskId = source.schoolDeskId;

	return *this;
}

BigSerial SchoolDesk::getSchoolDeskId() const
{
	return schoolDeskId;
}

void SchoolDesk::setSchoolDeskId(BigSerial schoolDeskId)
{
	this->schoolDeskId = schoolDeskId;
}

BigSerial SchoolDesk::getSchoolId() const
{
	return schoolId;
}

void SchoolDesk::setSchoolId(BigSerial schoolId)
{
	this->schoolId = schoolId;
}
