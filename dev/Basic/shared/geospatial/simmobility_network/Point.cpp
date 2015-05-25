//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Point.hpp"

using namespace simmobility_network;

Point::Point(unsigned int seqNum, double x, double y, double z) :
sequenceNumber(seqNum), x(x), y(y), z(z)
{
}

Point::Point(const Point& orig)
{
	this->sequenceNumber = orig.sequenceNumber;
	this->x = orig.x;
	this->y = orig.y;
	this->z = orig.z;
}

Point::~Point()
{
}

unsigned int Point::getSequenceNumber() const
{
	return sequenceNumber;
}

void Point::setSequenceNumber(unsigned int sequenceNumber)
{
	this->sequenceNumber = sequenceNumber;
}

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

