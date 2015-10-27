//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PolyLine.hpp"

using namespace sim_mob;

PolyLine::PolyLine() :
polyLineId(0), length(0)
{
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

const std::vector<PolyPoint>& PolyLine::getPoints() const
{
	return points;
}

const PolyPoint& PolyLine::getFirstPoint() const
{
	return points.front();
}

const PolyPoint& PolyLine::getLastPoint() const
{
	return points.back();
}

void PolyLine::addPoint(PolyPoint point)
{
	this->points.push_back(point);
}

size_t sim_mob::PolyLine::size() const
{
	return points.size();
}
