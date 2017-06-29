//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <string>
#include "Point.hpp"
#include "TurningGroup.hpp"
#include "spatial_trees/GeneralR_TreeManager.hpp"

namespace sim_mob
{
//aa{
const static unsigned TAZ_UNDEFINED = 0;
//aa}

class TurningGroup;
class Link;
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
class Person;
class Node
{
private:

	/**The unique identifier for a Node*/
	unsigned int nodeId;

	/**The location of the node*/
	Point location;

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

	std::map<unsigned int,Link*> mapOfDownStreamLinks;
	std::map<unsigned int,Link*> mapOfUpStreamLinks;
	std::vector<Person*> waitingPersons;

	//aa{
	/*
	 * The Traffic Analysis Zone (TAZ) to which this nodes belong
	 */
	unsigned int tazId;
	//aa}


public:

	Node();
	virtual ~Node();

	unsigned int getNodeId() const;
	void setNodeId(unsigned int nodeId);

	const Point& getLocation() const;
	void setLocation(Point location);

	NodeType getNodeType() const;
	void setNodeType(NodeType nodeType);

	unsigned int getTrafficLightId() const;
	void setTrafficLightId(unsigned int trafficLightId);

	std::map<unsigned int,Link*> getDownStreamLinks() const ;
	std::map<unsigned int,Link*> getUpStreamLinks();

	void addUpStreamLink(Link *link);
	void addDownStreamlink(Link *link);

	std::vector<Person*> personsWaitingForTaxi();

	std::vector<Node*> getNeighbouringNodes() const ;

    const std::map<unsigned int, std::map<unsigned int, TurningGroup *> >& getTurningGroups() const;

	/**
	 * This method adds a turning group to the map - turningGroups, based on the "from link" and "to link" of the
	 * turning group
	 *
	 * @param turningGroup - the turning group to be added
	 */
	void addTurningGroup(TurningGroup *turningGroup);

	/**
	 * This method looks up the turning group connecting the given from and to links and returns a pointer to it.
	 *
     * @param fromLinkId - the link id where the turning group begins
     * @param toLinkId - the link id where the turning group ends
	 *
     * @return the turning group if found, else NULL
     */
	const TurningGroup* getTurningGroup(unsigned int fromLinkId, unsigned int toLinkId) const;

	/**
	 * This method looks up the turning group originating at the given 'from link' and returns a map of
	 * turning groups with 'to link id' as the key
	 *
     * @param fromLinkId the id of the link from which the required turning groups must originate
	 *
     * @return pointer to the map of turning groups with 'to link id' as the key
     */
	const std::map<unsigned int, TurningGroup *>& getTurningGroups(unsigned int fromLinkId) const;

	/**store all nodes into a global r-tree*/
	static GeneralR_TreeManager<Node> allNodesMap;
	/**
	 * get x position
	 */
	double getPosX() const;
	/**
	 * get y position
	 */
	double getPosY() const;

	//aa{
	unsigned int getTazId()const;
	void setTazId(unsigned int tazId_);
	//aa}
};
}
