//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <stdexcept>
#include <sstream>
#include "RoadSegment.hpp"

using namespace sim_mob;

RoadSegment::RoadSegment() : roadSegmentId(0), capacity(0), linkId(0), maxSpeed(0), polyLine(NULL), sequenceNumber(0),
parentLink(NULL)
{
}

RoadSegment::~RoadSegment()
{
	for (std::vector<Lane *>::iterator itLane = lanes.begin(); itLane != lanes.end(); ++itLane)
	{
		delete *itLane;
		*itLane = NULL;
	}

	if (polyLine)
	{
		delete polyLine;
		polyLine = NULL;
	}
	
	for(std::map<double, RoadItem *>::iterator it = obstacles.begin(); it != obstacles.end(); it++)
	{
		delete it->second;
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

void RoadSegment::setCapacity(unsigned int capacity)
{
	this->capacity = capacity;
}

const std::vector<Lane *>& RoadSegment::getLanes() const
{
	return lanes;
}

const Lane* RoadSegment::getLane(int index) const
{
	return lanes.at(index);
}

unsigned int RoadSegment::getLinkId() const
{
	return linkId;
}

void RoadSegment::setLinkId(unsigned int linkId)
{
	this->linkId = linkId;
}

double RoadSegment::getMaxSpeed() const
{
	return maxSpeed;
}

void RoadSegment::setMaxSpeed(double maxSpeedKmph)
{
	this->maxSpeed = maxSpeedKmph / 3.6;
}

const Link* RoadSegment::getParentLink() const
{
	return parentLink;
}

void RoadSegment::setParentLink(Link *parentLink)
{
	this->parentLink = parentLink;
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

const std::map<double, RoadItem *>& RoadSegment::getObstacles() const
{
	return obstacles;
}

unsigned int RoadSegment::getNoOfLanes() const
{
	return lanes.size();
}

double RoadSegment::getLength() const
{
	return polyLine->getLength();
}

void RoadSegment::addLane(Lane *lane)
{
	this->lanes.push_back(lane);
}

void RoadSegment::addObstacle(double offset, RoadItem *item)
{
	if (offset < 0)
	{
		std::stringstream msg;
		msg << "Could not add obstacle " << item->getRoadItemId() << " to road segment " << this->roadSegmentId
			<< "\nOffset < 0";
		throw std::runtime_error(msg.str());
	}

	if (offset > this->getLength())
	{
		std::stringstream msg;
		msg << "Could not add obstacle " << item->getRoadItemId() << " to road segment " << this->roadSegmentId
			<< "\nOffset > Segment length";
		throw std::runtime_error(msg.str());
	}

	if (obstacles.count(offset) > 0)
	{
		std::stringstream msg;
		msg << "Could not add obstacle " << item->getRoadItemId() << " to road segment " << this->roadSegmentId
			<< "\nAnother obstacle at the same offset";
		throw std::runtime_error(msg.str());
	}

	obstacles[offset] = item;
}
