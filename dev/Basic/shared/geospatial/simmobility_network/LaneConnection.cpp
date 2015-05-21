//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "LaneConnection.hpp"

using namespace simmobility_network;

LaneConnection::LaneConnection(unsigned int id, unsigned int fromLane, unsigned int fromRoadSection, Tag *tag,
							   unsigned int toLane, unsigned int toRoadSection) :
laneConnectionId(id), fromLaneId(fromLane), fromRoadSectionId(fromRoadSection), tag(tag), toLaneId(toLane), toRoadSectionId(toRoadSection)
{
}

LaneConnection::LaneConnection(const LaneConnection& orig)
{
	this->laneConnectionId = orig.laneConnectionId;
	this->fromLaneId = orig.fromLaneId;
	this->fromRoadSectionId = orig.fromRoadSectionId;
	this->tag = orig.tag;
	this->toLaneId = orig.toLaneId;
	this->toRoadSectionId = orig.toRoadSectionId;
}

LaneConnection::~LaneConnection()
{
	if(tag)
	{
		delete tag;
		tag = NULL;
	}
}

unsigned int LaneConnection::getLaneConnectionId() const
{
	return laneConnectionId;
}

void LaneConnection::setLaneConnectionId(unsigned int laneConnectionId)
{
	this->laneConnectionId = laneConnectionId;
}

unsigned int LaneConnection::getFromLaneId() const
{
	return fromLaneId;
}

void LaneConnection::setFromLaneId(unsigned int fromLaneId)
{
	this->fromLaneId = fromLaneId;
}

unsigned int LaneConnection::getFromRoadSectionId() const
{
	return fromRoadSectionId;
}

void LaneConnection::setFromRoadSectionId(unsigned int fromRoadSectionId)
{
	this->fromRoadSectionId = fromRoadSectionId;
}

Tag* LaneConnection::getTag() const
{
	return tag;
}

void LaneConnection::setTag(Tag* tag)
{
	this->tag = tag;
}

unsigned int LaneConnection::getToLaneId() const
{
	return toLaneId;
}

void LaneConnection::setToLaneId(unsigned int toLaneId)
{
	this->toLaneId = toLaneId;
}

unsigned int LaneConnection::getToRoadSectionId() const
{
	return toRoadSectionId;
}

void LaneConnection::setToRoadSectionId(unsigned int toRoadSectionId)
{
	this->toRoadSectionId = toRoadSectionId;
}

