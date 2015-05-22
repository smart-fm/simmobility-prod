//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PolyLine.hpp"

using namespace simmobility_network;

PolyLine::PolyLine() :
polyLineId(0), type(POLYLINE_TYPE_POLYPOINT)
{
}

PolyLine::PolyLine(const PolyLine& orig)
{
	this->polyLineId = orig.polyLineId;
	this->points = orig.points;
	this->type = orig.type;
}

PolyLine::~PolyLine()
{
	//Delete the points in the poly-line
	for(std::vector<Point *>::iterator itPoints = points.begin(); itPoints != points.end(); ++itPoints)
	{
		delete *itPoints;
		*itPoints = NULL;
	}
}

int PolyLine::getPolyLineId() const
{
	return polyLineId;
}

void PolyLine::setPolyLineId(int polyLineId)
{
	this->polyLineId = polyLineId;
}

void PolyLine::setPoints(std::vector<Point*>& points)
{
	this->points = points;
}

const std::vector<Point*>& PolyLine::getPoints() const
{
	return points;
}

void PolyLine::setType(PolylineType type)
{
	this->type = type;
}

PolylineType PolyLine::getType() const
{
	return type;
}

