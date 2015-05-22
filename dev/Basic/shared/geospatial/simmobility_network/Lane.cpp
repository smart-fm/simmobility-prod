//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Lane.hpp"

using namespace simmobility_network;

Lane::Lane() :
laneId(0), laneIndex(0), laneRules(LANE_RULES_IS_PEDESTRIAN_LANE), polyLine(NULL), width(0)
{
}

Lane::Lane(const Lane& orig)
{
	this->laneId = orig.laneId;
	this->laneIndex = orig.laneIndex;
	this->laneRules = orig.laneRules;
	this->polyLine = orig.polyLine;	
	this->tags = orig.tags;
	this->width = orig.width;
}

Lane::~Lane()
{
	if(polyLine)
	{
		delete polyLine;
		polyLine = NULL;
	}
	tags.clear();
}

unsigned int Lane::getLaneId() const
{
	return laneId;
}

void Lane::setLaneId(unsigned int laneId)
{
	this->laneId = laneId;
}

unsigned int Lane::getLaneIndex() const
{
	return laneIndex;
}

void Lane::setLaneIndex(unsigned int laneIndex)
{
	this->laneIndex = laneIndex;
}

PolyLine* Lane::getPolyLine() const
{
	return polyLine;
}

void Lane::setPolyLine(PolyLine* polyLine)
{
	this->polyLine = polyLine;
}

const std::vector<Tag>& Lane::getTags() const
{
	return tags;
}

void Lane::setTags(std::vector<Tag>& tags)
{
	this->tags = tags;
}

double Lane::getWidth() const
{
	return width;
}

void Lane::setWidth(double width)
{
	this->width = width;
}