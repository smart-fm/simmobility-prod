/*
 * EzLinkStop.cpp
 *
 *  Created on: 13 Mar 2018
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

#include "EzLinkStop.hpp"

using namespace sim_mob::long_term;

EzLinkStop::EzLinkStop(BigSerial id , double xCoord, double yCoord, BigSerial nearestSchoolId): id(id),xCoord(xCoord),	yCoord(yCoord),nearestSchoolId(nearestSchoolId){}

EzLinkStop::~EzLinkStop(){}

EzLinkStop::EzLinkStop(const EzLinkStop& source)
{
	this->id = source.id;
	this->xCoord = source.xCoord;
	this->yCoord = source.yCoord;
	this->nearestSchoolId = source.nearestSchoolId;
}


EzLinkStop& EzLinkStop::operator=(const EzLinkStop& source)
{
	this->id = source.id;
	this->xCoord = source.xCoord;
	this->yCoord = source.yCoord;
	this->nearestSchoolId = source.nearestSchoolId;

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

BigSerial EzLinkStop::getNearestSchoolId() const
{
	return nearestSchoolId;
}

void EzLinkStop::setNearestSchoolId(BigSerial nearestSchoolId)
{
	this->nearestSchoolId = nearestSchoolId;
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
