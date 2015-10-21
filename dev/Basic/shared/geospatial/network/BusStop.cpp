//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusStop.hpp"
#include "Node.hpp"
#include "RoadSegment.hpp"
#include "Lane.hpp"
#include "util/LangHelpers.hpp"

using namespace sim_mob;

BusStop::BusStop() :
terminusType(NOT_A_TERMINUS), length(0.0), twinStop(nullptr), virtualStop(false), offset(0.0)
{
}

BusStop::~BusStop()
{
	safe_delete_item(twinStop);
	//busLines.clear();
}

const Point& BusStop::getStopLocation() const
{
	return location;
}

void BusStop::setStopLocation(Point& loc)
{
	location = loc;
}

const RoadSegment* BusStop::getParentSegment() const
{
	return roadSegment;
}

void BusStop::setParentSegment(RoadSegment *roadSegment)
{
	this->roadSegment = roadSegment;
}

const TerminusType& BusStop::getTerminusType() const
{
	return terminusType;
}

void BusStop::setTerminusType(TerminusType type)
{
	terminusType = type;
}

double BusStop::getCapacityAsLength() const
{
	return length;
}

void BusStop::setCapacityAsLength(double len)
{
	length = len;
}

/*
void BusStop::addBusLine(Busline& line)
{
	busLines.push_back(line);
}

const std::vector<Busline>& BusStop::getBusLine() const
{
	return busLines;
}
*/

const BusStop* BusStop::getTwinStop() const
{
	return twinStop;
}

void BusStop::setTwinStop(const BusStop* stop)
{
	twinStop = stop;
}

bool BusStop::isVirtualStop() const
{
	return virtualStop;
}

void BusStop::setVirtualStop(bool val)
{
	virtualStop = val;
}

double BusStop::getOffset() const
{
	return offset;
}

void BusStop::setOffset(double val)
{
	offset = val;
}

const std::string& BusStop::getStopName() const
{
	return stopName;
}

void BusStop::setStopName(std::string name)
{
	stopName = name;
}

unsigned int BusStop::getStopId() const
{
	return stopId;
}
void BusStop::setStopId(unsigned int id)
{
	stopId = id;
}

const std::string& BusStop::getStopCode() const
{
	return stopCode;
}
void BusStop::setStopCode(const std::string& code)
{
	stopCode = code;
}
