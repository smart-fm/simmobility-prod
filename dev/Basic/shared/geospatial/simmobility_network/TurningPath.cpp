//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "TurningPath.hpp"

using namespace simmobility_network;

TurningPath::TurningPath(unsigned int id, unsigned int fromLane, unsigned int geometryId, unsigned int polyLineId, Tag *tag,
						 unsigned int toLane, unsigned int turningGroupId) :
turningPathId(id), fromLaneId(fromLane), geometryId(geometryId), polyLineId(polyLineId), tag(tag), toLaneId(toLane),
turningGroupId(turningGroupId)
{
}

TurningPath::TurningPath(const TurningPath& orig)
{
	this->turningPathId = orig.turningPathId;
	this->fromLaneId = orig.fromLaneId;
	this->geometryId = orig.geometryId;
	this->polyLineId = orig.polyLineId;
	this->tag = orig.tag;
	this->toLaneId = orig.toLaneId;
	this->turningGroupId = orig.turningGroupId;
}

TurningPath::~TurningPath()
{
	if(tag)
	{
		delete tag;
		tag = NULL;
	}
}

unsigned int TurningPath::getTurningPathId() const
{
	return turningPathId;
}

void TurningPath::setTurningPathId(unsigned int turningPathId)
{
	this->turningPathId = turningPathId;
}

unsigned int TurningPath::getFromLaneId() const
{
	return fromLaneId;
}

void TurningPath::setFromLaneId(unsigned int fromLaneId)
{
	this->fromLaneId = fromLaneId;
}

unsigned int TurningPath::getGeometryId() const
{
	return geometryId;
}

void TurningPath::setGeometryId(unsigned int geometryId)
{
	this->geometryId = geometryId;
}

unsigned int TurningPath::getPolyLineId() const
{
	return polyLineId;
}

void TurningPath::setPolyLineId(unsigned int polyLineId)
{
	this->polyLineId = polyLineId;
}

Tag* TurningPath::getTag() const
{
	return tag;
}

void TurningPath::setTag(Tag* tag)
{
	this->tag = tag;
}

unsigned int TurningPath::getToLaneId() const
{
	return toLaneId;
}

void TurningPath::setToLaneId(unsigned int toLaneId)
{
	this->toLaneId = toLaneId;
}

unsigned int TurningPath::getTurningGroupId() const
{
	return turningGroupId;
}

void TurningPath::setTurningGroupId(unsigned int turningGroupId)
{
	this->turningGroupId = turningGroupId;
}
