//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RoadSection.hpp"

using namespace simmobility_network;

RoadSection::RoadSection(unsigned int id, unsigned int capacity, unsigned int geometryId, unsigned int linkId, unsigned int maxSpeed, unsigned int polyLineId,
						 std::string roadName, RoadSectionCategory category, RoadSectionType type, unsigned int seqNum, Tag *tag) :
roadSectionId(id), capacity(capacity), geometryId(geometryId), linkId(linkId), maxSpeed(maxSpeed), polyLineId(polyLineId), roadName(roadName),
roadSectionCategory(category), roadSectionType(type), sequenceNumber(seqNum), tag(tag)
{
}

RoadSection::RoadSection(const RoadSection& orig)
{
	this->roadSectionId = orig.roadSectionId;
	this->capacity = orig.capacity;
	this->geometryId = orig.geometryId;
	this->linkId = orig.linkId;
	this->maxSpeed = orig.maxSpeed;
	this->polyLineId = orig.polyLineId;
	this->roadName = orig.roadName;
	this->roadSectionCategory = orig.roadSectionCategory;
	this->roadSectionType = orig.roadSectionType;
	this->sequenceNumber = orig.sequenceNumber;
	this->tag = orig.tag;
}

RoadSection::~RoadSection()
{
	if(tag)
	{
		delete tag;
		tag = NULL;
	}
}

unsigned int RoadSection::getRoadSectionId() const
{
	return roadSectionId;
}

void RoadSection::setRoadSectionId(unsigned int roadSectionId)
{
	this->roadSectionId = roadSectionId;
}

unsigned int RoadSection::getCapacity() const
{
	return capacity;
}

void RoadSection::setCapacity(unsigned int capacity)
{
	this->capacity = capacity;
}

unsigned int RoadSection::getGeometryId() const
{
	return geometryId;
}

void RoadSection::setGeometryId(unsigned int geometryId)
{
	this->geometryId = geometryId;
}

unsigned int RoadSection::getLinkId() const
{
	return linkId;
}

void RoadSection::setLinkId(unsigned int linkId)
{
	this->linkId = linkId;
}

unsigned int RoadSection::getMaxSpeed() const
{
	return maxSpeed;
}

void RoadSection::setMaxSpeed(unsigned int maxSpeed)
{
	this->maxSpeed = maxSpeed;
}

unsigned int RoadSection::getPolyLineId() const
{
	return polyLineId;
}

void RoadSection::setPolyLineId(unsigned int polyLineId)
{
	this->polyLineId = polyLineId;
}

std::string RoadSection::getRoadName() const
{
	return roadName;
}

void RoadSection::setRoadName(std::string roadName)
{
	this->roadName = roadName;
}

RoadSectionCategory RoadSection::getRoadSectionCategory() const
{
	return roadSectionCategory;
}

void RoadSection::setRoadSectionCategory(RoadSectionCategory roadSectionCategory)
{
	this->roadSectionCategory = roadSectionCategory;
}

RoadSectionType RoadSection::getRoadSectionType() const
{
	return roadSectionType;
}

void RoadSection::setRoadSectionType(RoadSectionType roadSectionType)
{
	this->roadSectionType = roadSectionType;
}

unsigned int RoadSection::getSequenceNumber() const
{
	return sequenceNumber;
}

void RoadSection::setSequenceNumber(unsigned int sequenceNumber)
{
	this->sequenceNumber = sequenceNumber;
}

Tag* RoadSection::getTag() const
{
	return tag;
}

void RoadSection::setTag(Tag* tag)
{
	this->tag = tag;
}
