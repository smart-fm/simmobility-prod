//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RoadSegment.hpp"
#include "util/LangHelpers.hpp"

using namespace simmobility_network;

RoadSegment::RoadSegment() : roadSegmentId(0), capacity(0), linkId(0), maxSpeed(0), polyLine(NULL), sequenceNumber(0),
		tags(NULL), parentLink(NULL), length(0)
{}

RoadSegment::~RoadSegment()
{
	for(std::vector<Lane *>::iterator itLane = lanes.begin(); itLane != lanes.end(); ++itLane)
	{
		delete *itLane;
		*itLane = NULL;
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

const Lane* RoadSegment::getLane(int index) const
{
	return lanes.at(index);
}

void RoadSegment::setCapacity(unsigned int capacity)
{
	this->capacity = capacity;
}

const std::vector<Lane*>& RoadSegment::getLanes() const
{
	return lanes;
}

double RoadSegment::getLength()
{
	return polyLine->getLength();
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

const Link* RoadSegment::getParentLink() const
{
	return parentLink;
}

void RoadSegment::setParentLink(Link* parentLink)
{
	this->parentLink = parentLink;
}

const PolyLine* RoadSegment::getPolyLine() const
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
