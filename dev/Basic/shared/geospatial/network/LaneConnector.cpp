//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "LaneConnector.hpp"

using namespace sim_mob;

LaneConnector::LaneConnector() :
laneConnectionId(0), fromLaneId(0), fromRoadSegmentId(0), toLaneId(0), toRoadSegmentId(0), isTrueConnection(false), toLane(nullptr), fromLane(nullptr)
{
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

void LaneConnector::setFromLane(Lane *fromLane)
{
	this->fromLane = fromLane;
}

const Lane* LaneConnector::getFromLane() const
{
	return fromLane;
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

void LaneConnector::setToLane(Lane *toLane)
{
	this->toLane = toLane;
}

const Lane* LaneConnector::getToLane() const
{
	return toLane;
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

void LaneConnector::setIsTrueConnector(bool TrueConnector)
{
	isTrueConnection = TrueConnector;
}

bool LaneConnector::isTrueConnector() const
{
	return isTrueConnection;
}

