//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "LaneConnector.hpp"

using namespace simmobility_network;

LaneConnector::LaneConnector() :
laneConnectionId(0), fromLaneId(0), fromRoadSegmentId(0), toLaneId(0), toRoadSegmentId(0)
{
}

LaneConnector::LaneConnector(const LaneConnector& orig)
{
	this->laneConnectionId = orig.laneConnectionId;
	this->fromLaneId = orig.fromLaneId;
	this->fromRoadSegmentId = orig.fromRoadSegmentId;
	this->toLaneId = orig.toLaneId;
	this->toRoadSegmentId = orig.toRoadSegmentId;
}

LaneConnector::~LaneConnector()
{
}

unsigned int LaneConnector::getLaneConnectionId() const
{
	return laneConnectionId;
}

void LaneConnector::setLaneConnectionId(unsigned int laneConnectionId)
{
	this->laneConnectionId = laneConnectionId;
}

unsigned int LaneConnector::getFromLaneId() const
{
	return fromLaneId;
}

void LaneConnector::setFromLaneId(unsigned int fromLaneId)
{
	this->fromLaneId = fromLaneId;
}

unsigned int LaneConnector::getFromRoadSegmentId() const
{
	return fromRoadSegmentId;
}

void LaneConnector::setFromRoadSegmentId(unsigned int fromRoadSectionId)
{
	this->fromRoadSegmentId = fromRoadSectionId;
}

unsigned int LaneConnector::getToLaneId() const
{
	return toLaneId;
}

void LaneConnector::setToLaneId(unsigned int toLaneId)
{
	this->toLaneId = toLaneId;
}

unsigned int LaneConnector::getToRoadSegmentId() const
{
	return toRoadSegmentId;
}

void LaneConnector::setToRoadSegmentId(unsigned int toRoadSectionId)
{
	this->toRoadSegmentId = toRoadSectionId;
}

