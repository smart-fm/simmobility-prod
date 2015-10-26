//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Link.hpp"

using namespace sim_mob;

Link::Link() :
length(0), linkId(0), fromNode(NULL), fromNodeId(0), linkCategory(LINK_CATEGORY_A), linkType(LINK_TYPE_DEFAULT), roadName(""), toNode(NULL),
toNodeId(0)
{
}

Link::~Link()
{
	//Delete the road segment in the link
	for (std::vector<RoadSegment *>::iterator itSegments = roadSegments.begin(); itSegments != roadSegments.end(); ++itSegments)
	{
		delete *itSegments;
		*itSegments = NULL;
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

const Node* Link::getFromNode() const
{
	return fromNode;
}

void Link::setFromNode(Node *fromNode)
{
	this->fromNode = fromNode;
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

const RoadSegment* Link::getRoadSegment(int index) const
{
	return roadSegments.at(index);
}

const Node* Link::getToNode() const
{
	return toNode;
}

void Link::setToNode(Node *toNode)
{
	this->toNode = toNode;
}

unsigned int Link::getToNodeId() const
{
	return toNodeId;
}

void Link::setToNodeId(unsigned int toNodeId)
{
	this->toNodeId = toNodeId;
}

double Link::getLength() const
{	
	return length;
}

void Link::addRoadSegment(RoadSegment *roadSegment)
{
	this->roadSegments.push_back(roadSegment);
}

void Link::calculateLength()
{
	if(length == 0)
	{
		for (int i = 0; i < roadSegments.size(); ++i)
		{
			length += roadSegments.at(i)->getLength();
		}
	}
}