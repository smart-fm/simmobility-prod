//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RoadNetwork.hpp"
#include "Link.hpp"

using namespace simmobility_network;

RoadNetwork::RoadNetwork()
{
}

RoadNetwork::RoadNetwork(const RoadNetwork& orig)
{
	this->mapOfIdVsLinks = orig.mapOfIdVsLinks;
	this->mapOfIdvsNodes = orig.mapOfIdvsNodes;
}

RoadNetwork::~RoadNetwork()
{
	//Iterate through the map of nodes and delete the nodes
	std::map<unsigned int, Node *>::iterator itNodes = mapOfIdvsNodes.begin();
	while(itNodes != mapOfIdvsNodes.end())
	{
		delete itNodes->second;
		itNodes->second = NULL;
		++itNodes;
	}
	
	mapOfIdvsNodes.clear();
	
	//Iterate through the map of links and delete the links
	std::map<unsigned int, Link *>::iterator itLinks = mapOfIdVsLinks.begin();
	while(itLinks != mapOfIdVsLinks.end())
	{
		delete itLinks->second;
		itLinks->second = NULL;
		++itLinks;
	}
	
	mapOfIdVsLinks.clear();
}

void RoadNetwork::setMapOfIdvsNodes(std::map<unsigned int, Node*>& mapOfIdvsNodes)
{
	this->mapOfIdvsNodes = mapOfIdvsNodes;
}

const std::map<unsigned int, Node*>& RoadNetwork::getMapOfIdvsNodes() const
{
	return mapOfIdvsNodes;
}

void RoadNetwork::setMapOfIdVsLinks(std::map<unsigned int, Link*>& mapOfIdVsLinks)
{
	this->mapOfIdVsLinks = mapOfIdVsLinks;
}

const std::map<unsigned int, Link*>& RoadNetwork::getMapOfIdVsLinks() const
{
	return mapOfIdVsLinks;
}

