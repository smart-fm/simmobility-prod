#pragma once

#include <vector>
#include "Agent.hpp"
#include "../geospatial/Length.hpp"
#include "../util/DynamicVector.hpp"
#include "../constants.h"

namespace sim_mob{

class Node;
class Link;
class RoadSegment;
class BusStop;
class BusRoute;

class Bus : public sim_mob::Agent {

public:
	Bus(unsigned int id, BusRoute* busroute, BusStop* currentbusstop,
		Node* currentnode/*, DynamicVector heading, unsigned int distAlongHeading*/)
	{
		currBusStop = nullptr;
		lastBusStop = nullptr;
		destBusStop = nullptr;
		lastNode = nullptr;
		destNode = nullptr;
		currLinkEnd = nullptr;
	}

	virtual void update(frame_t frameNumber);
	virtual void buildSubscriptionList();
public:
	void updateAtNewStop(const BusStop* prevStop, const BusStop* nextStop);  ///<sets current bus stop to previous busstop and nextbus stop to current bus stop
	void updateAtNewNode(const Node* prevNode, const Node* newNode);  ///<same but for nodes
	void updateAtNewSegment();
	bool canMove(centimeter_t proposedX, centimeter_t proposedY); ///<checks if it can move to a given location

	const BusStop* currBusStop;
	const BusStop* lastBusStop;
	const BusStop* destBusStop;
	const Node* lastNode;
	const Node* destNode;


	std::vector<RoadSegment*>::iterator currSegment;
	std::vector<RoadSegment*> currSegmentList;
	const Node* currLinkEnd;
	DynamicVector heading;
	centimeter_t distAlongHeading;

};



}
