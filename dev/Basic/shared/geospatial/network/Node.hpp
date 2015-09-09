//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <string>

#include "Point.hpp"

namespace sim_mob
{
class TurningGroup;

/**Defines the various types of nodes supported by the SimMobility network*/
enum NodeType
{
	/**Default node*/
	DEFAULT_NODE = 0,

	/**
	 * Source node - only has outgoing road sections connected to it
	 * Sink node - only has incoming road sections connected to it
	 */
	SOURCE_OR_SINK_NODE = 1,

	/**This type of node is an intersection with or without a traffic signal*/
	INTERSECTION_NODE = 2,

	/**This type of node is a merging node*/
	MERGE_NODE = 3
};

/**
 * Defines the structure of a Node
 * \author Neeraj D
 * \author Harish L
 */
class Node
{
private:

	/**The unique identifier for a Node*/
	unsigned int nodeId;

	/**The location of the node*/
	Point* location;

	/**The type of the node*/
	NodeType nodeType;

	/**
	 * The identifier for the traffic light if present at the node
	 * A non-zero value indicates the presence of a traffic light
	 */
	unsigned int trafficLightId;

	/**
	 * The turning groups located at the node. The outer map stores the "from link id" vs
	 * an inner map, which store the "to link id" vs the turning group
	 */
	std::map<unsigned int, std::map<unsigned int, TurningGroup *> > turningGroups;

public:

	Node();

	virtual ~Node();

	void setNodeId(unsigned int nodeId);
	unsigned int getNodeId() const;

	void setLocation(Point *location);
	Point* getLocation() const;

	void setNodeType(NodeType nodeType);
	NodeType getNodeType() const;

	void setTrafficLightId(unsigned int trafficLightId);
	unsigned int getTrafficLightId() const;

	/**
	 * This method adds a turning group to the map - turningGroups, based on the "from link" and "to link" of the
	 * turning group
	 * @param turningGroup - the turning group to be added
	 */
	void addTurningGroup(TurningGroup *turningGroup);
};
}
