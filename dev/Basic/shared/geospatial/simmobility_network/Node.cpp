//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Node.hpp"

using namespace simmobility_network;

Node::Node() :
nodeId(0), location(NULL), nodeType(DEFAULT_NODE), trafficLightId(0)
{
}

Node::Node(const Node& orig)
{
	this->nodeId = orig.nodeId;
	this->location = orig.location;
	this->nodeType = orig.nodeType;	
	this->tags = orig.tags;
	this->trafficLightId = orig.trafficLightId;
	this->turningGroups = orig.turningGroups;
}

Node::~Node()
{
	//Delete the point storing the location
	if(location)
	{
		delete location;
		location = NULL;
	}

	tags.clear();
	
	//Delete the turning groups
	
	//Iterate through the outer map
	std::map<unsigned int, std::map<unsigned int, TurningGroup *> >::iterator itOuterMap = turningGroups.begin();
	while(itOuterMap != turningGroups.end())
	{
		//Iterate through the inner map
		std::map<unsigned int, TurningGroup *>::iterator itInnerMap = itOuterMap->second.begin();
		while(itInnerMap != itOuterMap->second.end())
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

void Node::setTags(std::vector<Tag>& tags)
{
	this->tags = tags;
}

const std::vector<Tag>& Node::getTags() const
{
	return tags;
}

void Node::setTrafficLightId(unsigned int trafficLightId)
{
	this->trafficLightId = trafficLightId;
}

unsigned int Node::getTrafficLightId() const
{
	return trafficLightId;
}