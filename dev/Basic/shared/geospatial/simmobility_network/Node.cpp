//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Node.hpp"
#include "util/LangHelpers.hpp"

using namespace simmobility_network;

Node::Node(unsigned int id, NodeType type, Tag *tag, unsigned int trafficLightId) :
nodeId(id), nodeType(type), tag(tag), trafficLightId(trafficLightId)
{
}

Node::Node(const Node& orig)
{
	this->nodeId = orig.nodeId;
	this->nodeType = orig.nodeType;
	this->tag = orig.tag;
	this->trafficLightId = orig.trafficLightId;
}

Node::~Node()
{
	if(tag)
	{
		delete tag;
		tag = nullptr;
	}
}

void Node::setNodeId(unsigned int nodeId)
{
	this->nodeId = nodeId;
}

unsigned int Node::getNodeId() const
{
	return nodeId;
}

void Node::setNodeType(NodeType nodeType)
{
	this->nodeType = nodeType;
}

NodeType Node::getNodeType() const
{
	return nodeType;
}

void Node::setTag(Tag *tag)
{
	this->tag = tag;
}

Tag* Node::getTag() const
{
	return tag;
}

void Node::setTrafficLightId(unsigned int trafficLightId)
{
	this->trafficLightId = trafficLightId;
}

unsigned int Node::getTrafficLightId() const
{
	return trafficLightId;
}