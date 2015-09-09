//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Point.hpp"

using namespace sim_mob;

Point::Point() :
		x(0), y(0), z(0)
{}

Point::Point(double x, double y, double z) :
		x(x), y(y), z(z)
{}

Point::~Point()
{}

double Point::getX() const
{
	return x;
}

void Point::setX(double x)
{
	this->x = x;
}

double Point::getY() const
{
	return y;
}

void Point::setY(double y)
{
	this->y = y;
}

double Point::getZ() const
{
	return z;
}

void Point::setZ(double z)
{
	this->z = z;
}

PolyPoint::PolyPoint() :
		Point(), polyLineId(0), sequenceNumber(0)
{}

PolyPoint::PolyPoint(unsigned int id, unsigned int seqNum, double x, double y, double z) :
		Point(x,y,z), polyLineId(id), sequenceNumber(seqNum)
{}

PolyPoint::~PolyPoint()
{}

unsigned int PolyPoint::getPolyLineId() const
{
	return polyLineId;
}

void PolyPoint::setPolyLineId(unsigned int polyLineId)
{
	this->polyLineId = polyLineId;
}

unsigned int PolyPoint::getSequenceNumber() const
{
	return sequenceNumber;
}

void PolyPoint::setSequenceNumber(unsigned int sequenceNumber)
{
	this->sequenceNumber = sequenceNumber;
}
