//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RoadSegment.hpp"

using namespace simmobility_network;

RoadSegment::RoadSegment() :
roadSegmentId(0), capacity(0), maxSpeed(0), polyLine(NULL), roadName(0), sequenceNumber(0)
{
}

RoadSegment::RoadSegment(const RoadSegment& orig)
{
	this->roadSegmentId = orig.roadSegmentId;
	this->capacity = orig.capacity;
	this->maxSpeed = orig.maxSpeed;
	this->polyLine = orig.polyLine;
	this->roadName = orig.roadName;
	this->sequenceNumber = orig.sequenceNumber;
	this->tags = orig.tags;
}

RoadSegment::~RoadSegment()
{
	if(polyLine)
	{
		delete polyLine;
		polyLine = NULL;
	}
	
	tags.clear();
}

unsigned int RoadSegment::getRoadSectionId() const
{
	return roadSegmentId;
}

void RoadSegment::setRoadSectionId(unsigned int roadSectionId)
{
	this->roadSegmentId = roadSectionId;
}

unsigned int RoadSegment::getCapacity() const
{
	return capacity;
}

void RoadSegment::setCapacity(unsigned int capacity)
{
	this->capacity = capacity;
}

unsigned int RoadSegment::getMaxSpeed() const
{
	return maxSpeed;
}

void RoadSegment::setMaxSpeed(unsigned int maxSpeed)
{
	this->maxSpeed = maxSpeed;
}

PolyLine* RoadSegment::getPolyLine() const
{
	return polyLine;
}

void RoadSegment::setPolyLine(PolyLine *polyLine)
{
	this->polyLine = polyLine;
}

std::string RoadSegment::getRoadName() const
{
	return roadName;
}

void RoadSegment::setRoadName(std::string roadName)
{
	this->roadName = roadName;
}

unsigned int RoadSegment::getSequenceNumber() const
{
	return sequenceNumber;
}

void RoadSegment::setSequenceNumber(unsigned int sequenceNumber)
{
	this->sequenceNumber = sequenceNumber;
}

const std::vector<Tag>& RoadSegment::getTags() const
{
	return tags;
}

void RoadSegment::setTag(std::vector<Tag>& tags)
{
	this->tags = tags;
}
