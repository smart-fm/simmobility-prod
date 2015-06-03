//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RoadSegment.hpp"

using namespace simmobility_network;

RoadSegment::RoadSegment() :
roadSegmentId(0), capacity(0), linkId(0), maxSpeed(0), polyLine(NULL), sequenceNumber(0), tags(NULL),parentLink(NULL)
{
}

RoadSegment::RoadSegment(const RoadSegment& orig)
{
	this->roadSegmentId = orig.roadSegmentId;
	this->capacity = orig.capacity;
	this->lanes = orig.lanes;
	this->linkId = orig.linkId;
	this->maxSpeed = orig.maxSpeed;
	this->polyLine = orig.polyLine;
	this->sequenceNumber = orig.sequenceNumber;
	this->tags = orig.tags;
	this->parentLink = orig.parentLink;
}

RoadSegment::~RoadSegment()
{
	//Delete the lanes in the road segment
	for(std::vector<Lane *>::iterator itLanes = lanes.begin(); itLanes != lanes.end(); ++itLanes)
	{
		delete *itLanes;
		*itLanes = NULL;
	}
	
	if(polyLine)
	{
		delete polyLine;
		polyLine = NULL;
	}
	
	if(tags)
	{
		delete tags;
		tags = NULL;
	}
}

double RoadSegment::getLength()
{
	return polyLine->getLength();
}
unsigned int RoadSegment::getRoadSegmentId() const
{
	return roadSegmentId;
}

void RoadSegment::setRoadSegmentId(unsigned int roadSegmentId)
{
	this->roadSegmentId = roadSegmentId;
}

unsigned int RoadSegment::getCapacity() const
{
	return capacity;
}

void RoadSegment::setCapacity(unsigned int capacity)
{
	this->capacity = capacity;
}

const std::vector<Lane*>& RoadSegment::getLanes() const
{
	return lanes;
}

unsigned int RoadSegment::getLinkId() const
{
	return linkId;
}

void RoadSegment::setLinkId(unsigned int linkId)
{
	this->linkId = linkId;
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

unsigned int RoadSegment::getSequenceNumber() const
{
	return sequenceNumber;
}

void RoadSegment::setSequenceNumber(unsigned int sequenceNumber)
{
	this->sequenceNumber = sequenceNumber;
}

const std::vector<Tag>* RoadSegment::getTags() const
{
	return tags;
}

void RoadSegment::setTags(std::vector<Tag> *tags)
{
	this->tags = tags;
}

void RoadSegment::addLane(Lane *lane)
{
	this->lanes.push_back(lane);
}
