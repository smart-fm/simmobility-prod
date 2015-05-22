//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Link.hpp"

using namespace simmobility_network;

Link::Link() :
linkId(0), fromNodeId(0), linkCategory(LINK_CATEGORY_DEFAULT), roadName(""), toNodeId(0)
{
}

Link::Link(const Link& orig)
{
	this->linkId = orig.linkId;
	this->fromNodeId = orig.fromNodeId;
	this->linkCategory = orig.linkCategory;
	this->roadName = orig.roadName;
	this->tags = orig.tags;
	this->toNodeId = orig.toNodeId;
}

Link::~Link()
{
	tags.clear();
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

std::string Link::getRoadName() const
{
	return roadName;
}

void Link::setRoadName(std::string roadName)
{
	this->roadName = roadName;
}

const std::vector<Tag>& Link::getTags() const
{
	return tags;
}

void Link::setTags(std::vector<Tag>& tags)
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

