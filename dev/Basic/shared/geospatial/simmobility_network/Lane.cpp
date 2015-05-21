//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Lane.hpp"

using namespace simmobility_network;

Lane::Lane(unsigned int id, unsigned int geometryId, unsigned int index, unsigned int sectionId, Tag *tag, unsigned int pathId) :
laneId(id), geometryId(geometryId), laneIndex(index), roadSectionId(sectionId), tag(tag), turningPathId(pathId)
{
}

Lane::Lane(const Lane& orig)
{
	this->laneId = orig.laneId;
	this->geometryId = orig.geometryId;
	this->laneIndex = orig.laneIndex;
	this->roadSectionId = orig.roadSectionId;
	this->tag = orig.tag;
	this->turningPathId = orig.turningPathId;
}

Lane::~Lane()
{
	if(tag)
	{
		delete tag;
		tag = NULL;
	}
}

unsigned int Lane::getLaneId() const
{
	return laneId;
}

void Lane::setLaneId(unsigned int laneId)
{
	this->laneId = laneId;
}

unsigned int Lane::getGeometryId() const
{
	return geometryId;
}

void Lane::setGeometryId(unsigned int geometryId)
{
	this->geometryId = geometryId;
}

unsigned int Lane::getLaneIndex() const
{
	return laneIndex;
}

void Lane::setLaneIndex(unsigned int laneIndex)
{
	this->laneIndex = laneIndex;
}

LaneType Lane::getLaneType() const
{
	return laneType;
}

void Lane::setLaneType(LaneType laneType)
{
	this->laneType = laneType;
}

unsigned int Lane::getRoadSectionId() const
{
	return roadSectionId;
}

void Lane::setRoadSectionId(unsigned int roadSectionId)
{
	this->roadSectionId = roadSectionId;
}

Tag* Lane::getTag() const
{
	return tag;
}

void Lane::setTag(Tag* tag)
{
	this->tag = tag;
}

unsigned int Lane::getTurningPathId() const
{
	return turningPathId;
}

void Lane::setTurningPathId(unsigned int turningPathId)
{
	this->turningPathId = turningPathId;
}

