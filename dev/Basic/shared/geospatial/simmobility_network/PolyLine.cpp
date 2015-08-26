//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PolyLine.hpp"

using namespace simmobility_network;

PolyLine::PolyLine() :
polyLineId(0), length(0)
{
}

PolyLine::PolyLine(const PolyLine& orig)
{
	this->polyLineId = orig.polyLineId;
	this->length = orig.length;
	this->points = orig.points;
}

PolyLine::~PolyLine()
{
}

int PolyLine::getPolyLineId() const
{
	return polyLineId;
}

void PolyLine::setPolyLineId(int polyLineId)
{
	this->polyLineId = polyLineId;
}

void PolyLine::setLength(double length)
{
	this->length = length;
}

double PolyLine::getLength() const
{
	return length;
}

const std::vector<Point>& PolyLine::getPoints() const
{
	return points;
}

void PolyLine::addPoint(Point point)
{
	this->points.push_back(point);
}

Point PolyLine::getFirstPoint() const
{
	return points[0];
}

Point PolyLine::getLastPoint() const
{
	return points.at(points.size()-1);
}
