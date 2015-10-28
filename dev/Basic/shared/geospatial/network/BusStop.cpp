//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusStop.hpp"
#include "Node.hpp"
#include "RoadSegment.hpp"
#include "Lane.hpp"
#include "util/LangHelpers.hpp"

using namespace sim_mob;

std::map<std::string, BusStop *> BusStop::mapOfCodevsBusStops;

BusStop::BusStop() :
		terminusType(NOT_A_TERMINUS), length(0.0), twinStop(nullptr), virtualStop(false), offset(0.0),
		reverseSectionId(0), terminalNodeId(0), roadSegment(NULL), stopCode(std::string())
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

void BusStop::setStopLocation(Point loc)
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

void BusStop::setLength(double length)
{
	this->length = length;
}

double BusStop::getLength() const
{
	return length;
}

/*
void BusStop::addBusLine(Busline &line)
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

void BusStop::setTwinStop(const BusStop *stop)
{
	twinStop = stop;
}

bool BusStop::isVirtualStop() const
{
	return virtualStop;
}

void BusStop::setVirtualStop()
{
	virtualStop = true;
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

const std::string& BusStop::getStopCode() const
{
	return stopCode;
}

void BusStop::setStopCode(const std::string& code)
{
	stopCode = code;
}

void BusStop::registerBusStop(BusStop *stop)
{
	std::string code = stop->getStopCode();
	if (mapOfCodevsBusStops.find(code) == mapOfCodevsBusStops.end())
	{
		mapOfCodevsBusStops[code] = stop;
	}
}

BusStop* BusStop::findBusStop(const std::string& code)
{
	BusStop *stop = nullptr;
	std::map<std::string, BusStop *>::iterator it = mapOfCodevsBusStops.find(code);
	
	if (it != mapOfCodevsBusStops.end())
	{
		stop = it->second;
	}
	
	return stop;
}

unsigned int BusStop::getReverseSectionId() const
{
	return reverseSectionId;
}

void BusStop::setReverseSectionId(unsigned int reverseSectionId)
{
	this->reverseSectionId = reverseSectionId;
}

unsigned int BusStop::getTerminalNodeId() const
{
	return terminalNodeId;
}

void BusStop::setTerminalNodeId(unsigned int terminalNodeId)
{
	this->terminalNodeId = terminalNodeId;
}
