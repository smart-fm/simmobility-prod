//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Link.hpp"

using namespace simmobility_network;

Link::Link() :
linkId(0), fromNodeId(0), linkCategory(LINK_CATEGORY_DEFAULT), linkType(LINK_TYPE_DEFAULT), roadName(""), tags(NULL), toNodeId(0),
fromNode(NULL),toNode(NULL)
{
}

Link::Link(const Link& orig)
{
	this->linkId = orig.linkId;
	this->fromNodeId = orig.fromNodeId;
	this->linkCategory = orig.linkCategory;
	this->linkType = orig.linkType;
	this->roadName = orig.roadName;
	this->roadSegments = orig.roadSegments;
	this->tags = orig.tags;
	this->toNodeId = orig.toNodeId;
	this->fromNode = orig.fromNode;
	this->toNode = orig.toNode;
}

Link::~Link()
{
	//Delete the road segment in the link
	for(std::vector<RoadSegment *>::iterator itSegments = roadSegments.begin(); itSegments != roadSegments.end(); ++itSegments)
	{
		delete *itSegments;
		*itSegments = NULL;
	}
	
	if(tags)
	{
		delete tags;
		tags = NULL;
	}
}
double Link::getLength()
{
	length = 0;
	for(int i=0;i<roadSegments.size();++i){
		length += roadSegments.at(i)->getLength();
	}
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

LinkCategory Link::getLinkCategory() const
{
	return linkCategory;
}

void Link::setLinkCategory(LinkCategory linkCategory)
{
	this->linkCategory = linkCategory;
}

LinkType Link::getLinkType() const
{
	return linkType;
}

void Link::setLinkType(LinkType linkType)
{
	this->linkType = linkType;
}

std::string Link::getRoadName() const
{
	return roadName;
}

void Link::setRoadName(std::string roadName)
{
	this->roadName = roadName;
}

const std::vector<RoadSegment*>& Link::getRoadSegments() const
{
	return roadSegments;
}

const std::vector<Tag>* Link::getTags() const
{
	return tags;
}

void Link::setTags(std::vector<Tag> *tags)
{
	this->tags = tags;
}

unsigned int Link::getToNodeId() const
{
	return toNodeId;
}

void Link::setToNodeId(unsigned int toNodeId)
{
	this->toNodeId = toNodeId;
}

void Link::addRoadSegment(RoadSegment *roadSegment)
{
	this->roadSegments.push_back(roadSegment);
}
