/*
 * EzLinkStop.cpp
 *
 *  Created on: 13 Mar 2018
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

#include "EzLinkStop.hpp"

using namespace sim_mob::long_term;

EzLinkStop::EzLinkStop(BigSerial id , double xCoord, double yCoord, BigSerial nearestUniversityId, BigSerial nearestPolytechnicId): id(id),xCoord(xCoord),	yCoord(yCoord),nearestUniversityId(nearestUniversityId), nearestPolytechnicId(nearestPolytechnicId){}

EzLinkStop::~EzLinkStop(){}

EzLinkStop::EzLinkStop(const EzLinkStop& source)
{
	this->id = source.id;
	this->xCoord = source.xCoord;
	this->yCoord = source.yCoord;
	this->nearestUniversityId = source.nearestUniversityId;
	this->nearestPolytechnicId = source.nearestPolytechnicId;
}


EzLinkStop& EzLinkStop::operator=(const EzLinkStop& source)
{
	this->id = source.id;
	this->xCoord = source.xCoord;
	this->yCoord = source.yCoord;
	this->nearestUniversityId = source.nearestUniversityId;
	this->nearestPolytechnicId = source.nearestPolytechnicId;

	return *this;
}

BigSerial EzLinkStop::getId() const
{
	return id;
}

void EzLinkStop::setId(BigSerial id)
{
	this->id = id;
}

double EzLinkStop::getXCoord() const
{
	return xCoord;
}

void EzLinkStop::setXCoord(double coord)
{
	xCoord = coord;
}

double EzLinkStop::getYCoord() const
{
	return yCoord;
}

void EzLinkStop::setYCoord(double coord)
{
	yCoord = coord;
}

BigSerial EzLinkStop::getNearestPolytechnicId() const
{
	return nearestPolytechnicId;
}

void EzLinkStop::setNearestPolytechnicId(BigSerial nearestPolytechnicId)
{
	this->nearestPolytechnicId = nearestPolytechnicId;
}

BigSerial EzLinkStop::getNearestUniversityId() const
{
	return nearestUniversityId;
}

void EzLinkStop::setNearestUniversityId(BigSerial nearestUniversityId)
{
	this->nearestUniversityId = nearestUniversityId;
}
