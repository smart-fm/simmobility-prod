//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Node.hpp"
#include "TurningGroup.hpp"

using namespace sim_mob;

Node::Node() :
nodeId(0), location(NULL), nodeType(DEFAULT_NODE), trafficLightId(0)
{
}

Node::~Node()
{
	//Delete the point storing the location
	if (location)
	{
		delete location;
		location = NULL;
	}

	//Delete the turning groups

	//Iterate through the outer map
	std::map<unsigned int, std::map<unsigned int, TurningGroup *> >::iterator itOuterMap = turningGroups.begin();
	while (itOuterMap != turningGroups.end())
	{
		//Iterate through the inner map
		std::map<unsigned int, TurningGroup *>::iterator itInnerMap = itOuterMap->second.begin();
		while (itInnerMap != itOuterMap->second.end())
		{
			//Delete the turning group
			delete itInnerMap->second;
			itInnerMap->second = NULL;
			++itInnerMap;
		}

		//Clear the inner map
		itOuterMap->second.clear();

		++itOuterMap;
	}

	//Clear the map
	turningGroups.clear();
}

void Node::setNodeId(unsigned int nodeId)
{
	this->nodeId = nodeId;
}

unsigned int Node::getNodeId() const
{
	return nodeId;
}

void Node::setLocation(Point* location)
{
	this->location = location;
}

Point* Node::getLocation() const
{
	return location;
}

void Node::setNodeType(NodeType nodeType)
{
	this->nodeType = nodeType;
}

NodeType Node::getNodeType() const
{
	return nodeType;
}

void Node::setTrafficLightId(unsigned int trafficLightId)
{
	this->trafficLightId = trafficLightId;
}

unsigned int Node::getTrafficLightId() const
{
	return trafficLightId;
}

void Node::addTurningGroup(TurningGroup* turningGroup)
{
	//Find the map entry having the key given by the "from link" of the turning group
	std::map<unsigned int, std::map<unsigned int, TurningGroup *> >::iterator itOuter = turningGroups.find(turningGroup->getFromLinkId());

	//Check if such an entry exists
	if (itOuter != turningGroups.end())
	{
		//Entry found, so just add a new entry in the inner map using the "to link" as the key
		itOuter->second.insert(std::make_pair(turningGroup->getToLinkId(), turningGroup));
	}
	//Inner map doesn't exist
	else
	{
		//Create the inner map
		std::map<unsigned int, TurningGroup *> innerMap;

		//Make a new entry in the inner map
		innerMap.insert(std::make_pair(turningGroup->getToLinkId(), turningGroup));

		//Make a new entry into the outer map
		turningGroups.insert(std::make_pair(turningGroup->getFromLinkId(), innerMap));
	}
}
