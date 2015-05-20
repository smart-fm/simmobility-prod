//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Link.hpp"

using namespace simmobility_network;

Link::Link(unsigned int id, unsigned int fromNode, unsigned int geometryId, unsigned int polyLineId, std::string roadName,
		   Tag *tag, unsigned int toNode) :
linkId(id), fromNodeId(fromNode), geometryId(geometryId), polyLineId(polyLineId), roadName(roadName), tag(tag), toNodeId(toNode)
{
}

Link::Link(const Link& orig)
{
	this->linkId = orig.linkId;
	this->fromNodeId = orig.fromNodeId;
	this->geometryId = orig.geometryId;
	this->polyLineId = orig.polyLineId;
	this->roadName = orig.roadName;
	this->tag = orig.tag;
	this->toNodeId = orig.toNodeId;
}

Link::~Link()
{
}

unsigned int Link::getLinkId() const
{
	return linkId;
}

void Link::setLinkId(unsigned int linkId)
{
	this->linkId = linkId;
}

unsigned int Link::getFromNodeId() const
{
	return fromNodeId;
}

void Link::setFromNodeId(unsigned int fromNodeId)
{
	this->fromNodeId = fromNodeId;
}

unsigned int Link::getGeometryId() const
{
	return geometryId;
}

void Link::setGeometryId(unsigned int geometryId)
{
	this->geometryId = geometryId;
}

unsigned int Link::getPolyLineId() const
{
	return polyLineId;
}

void Link::setPolyLineId(unsigned int polyLineId)
{
	this->polyLineId = polyLineId;
}

std::string Link::getRoadName() const
{
	return roadName;
}

void Link::setRoadName(std::string roadName)
{
	this->roadName = roadName;
}

Tag* Link::getTag() const
{
	return tag;
}

void Link::setTag(Tag* tag)
{
	this->tag = tag;
}

unsigned int Link::getToNodeId() const
{
	return toNodeId;
}

void Link::setToNodeId(unsigned int toNodeId)
{
	this->toNodeId = toNodeId;
}

