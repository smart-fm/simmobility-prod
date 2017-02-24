/*
 * Job.cpp
 *
 *  Created on: 23 Apr, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/Job.hpp>

using namespace sim_mob::long_term;

Job::Job(BigSerial id, BigSerial establishmentId):id(id), establishmentId(establishmentId){}

Job::~Job() {}

void Job::setId(BigSerial val)
{
	id = val;
}

void Job::setEstablishmentId(BigSerial val)
{
	establishmentId = val;
}


BigSerial Job::getId() const
{
	return id;
}

BigSerial Job::getEstablishmentId() const
{
	return establishmentId;
}





