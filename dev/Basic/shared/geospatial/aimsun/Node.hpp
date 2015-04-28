//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <cmath>
#include <map>

#include "util/LangHelpers.hpp"

#include "Base.hpp"

namespace sim_mob
{

//Forward declarations
class Node;
class Intersection;

struct NodeType
{
	std::string id;
	int type;
};

namespace aimsun
{

//Forward declarations
class Section;
class Crossing;


///An AIMSUN road intersection or segment intersection.
/// \author Seth N. Hetu
class Node : public Base
{
public:
	double xPos;
	double yPos;
	std::string nodeName;
	bool isIntersection;
	bool hasTrafficSignal;

	//Decorated data
	std::vector<Section*> sectionsAtNode;
	std::map<int, std::vector<Crossing*> > crossingsAtNode; //Arranged by laneID
	std::map<Node*, std::vector<int> > crossingLaneIdsByOutgoingNode;
	bool candidateForSegmentNode;

	//Reference to saved object (Maybe be UniNode or MultiNode, of course)
	sim_mob::Node* generatedNode;

	Node() : Base(), xPos(0), yPos(0), nodeName(""), isIntersection(false), candidateForSegmentNode(false), generatedNode(nullptr), hasTrafficSignal(false) {}
	int getXPosAsInt() { return round(xPos); }
	int getYPosAsInt() { return round(yPos); }
};
}
}
