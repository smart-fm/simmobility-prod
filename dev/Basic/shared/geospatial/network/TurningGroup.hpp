//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include "TurningPath.hpp"

namespace sim_mob
{

class TurningPath;

/**Defines the rules vehicles must observe at all the turnings in the same group*/
enum TurningGroupRule
{
	/**No stop sign at the turning group*/
	TURNING_GROUP_RULE_NO_STOP_SIGN = 0,

	/**Stop sign present at the turning group*/
	TURNING_GROUP_RULE_STOP_SIGN = 1
};

/**
 * A turning group is a group of turning paths that connect the same links across a node.
 * This class defines the structure of a turning group.
 * \author Neeraj D
 * \author Harish L
 */
class TurningGroup
{
public:

	/**Unique identifier for the turning group*/
	unsigned int turningGroupId;

	/**Indicates the link from which this turning group originates*/
	unsigned int fromLinkId;

	/**The id of the node to which this turning group belongs*/
	unsigned int nodeId;

	/**Indicates the phases of the traffic light during which the vehicles can pass*/
	std::string phases;

	/**Stores the turning group rules*/
	TurningGroupRule groupRule;

	/**Indicates the link at which this turning group terminates*/
	unsigned int toLinkId;

	/**
	 * The turning paths located in a turning group. The map stores the 'from lane id' as the key and
	 * an inner map with the 'to lane id' as the key and the turning path as the value
	 */
	std::map<unsigned int, std::map<unsigned int, TurningPath *> > turningPaths;

	/**Defines the visibility of the intersection from the turning group (m/s)*/
	double visibility;

	/**The total number of turning paths that are in this turning group*/
	unsigned int numTurningPaths;

	/**The length of the turning group. This is an average of the lengths of the turning paths that belong to the turning group*/
	double length;

public:
	TurningGroup();
	virtual ~TurningGroup();

	unsigned int getTurningGroupId() const;
	void setTurningGroupId(unsigned int turningGroupId);

	unsigned int getFromLinkId() const;
	void setFromLinkId(unsigned int fromLinkId);

	unsigned int getNodeId() const;
	void setNodeId(unsigned int nodeId);

	std::string getPhases() const;
	void setPhases(std::string phases);

	TurningGroupRule getRule() const;
	void setRule(TurningGroupRule rules);

	unsigned int getToLinkId() const;
	void setToLinkId(unsigned int toLinkId);

	double getVisibility() const;
	void setVisibility(double visibility);

	const std::map<unsigned int, std::map<unsigned int, TurningPath *> >& getTurningPaths() const;

	unsigned int getNumTurningPaths() const;
	double getLength() const;

	/**
	 * Adds the turning path into the map of turningPaths
	 * @param turningPath - turning path to be added to the turning group
	 */
	void addTurningPath(TurningPath *turningPath);

	/**
	 * This method looks up the turning paths from the given lane and returns map of with the destination lane as key and
	 * the turning path as the value.
	 * 
	 * @param fromLaneId - the lane id where the turning path begins
	 *
	 * @return the map of "to lane id" vs turning path if found, else NULL
	 */
	const std::map<unsigned int, TurningPath *>* getTurningPaths(unsigned int fromLaneId) const;
};
}
