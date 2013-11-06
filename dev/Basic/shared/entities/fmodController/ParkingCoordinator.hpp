//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ParkingCoordinator.h
 *
 *  Created on: Jul 3, 2013
 *      Author: zhang
 */

#ifndef PARKINGCOORDINATOR_H_
#define PARKINGCOORDINATOR_H_

#include "vector"
#include "map"

namespace sim_mob {

class Node;
class Agent;


namespace FMOD
{

/**
  * provide a virtual parking mechanism for demanded drivers
  */
class ParkingCoordinator {
public:
	ParkingCoordinator();
	virtual ~ParkingCoordinator();

	/**
	  * data structure to store parking vehicles at a given node
	  */
	struct ParkingLot{
		ParkingLot() : node(0), maxCapacity(10), currentOccupancy(0) {;}
		const Node* node;
		int maxCapacity;
		int currentOccupancy;
		std::vector<const Agent*> vehicles;
	};

    /**
      * driver enter a node for parking
      * @param node is parking location
      * @param agent is the driver who want to park
      * @return true if successfully, otherwise retrun false
      */
	bool enterTo(const Node* node, const Agent* agent);

    /**
      * driver leave from a node for a new trip
      * @param node is parking location
      * @param agent is the driver who want to leave
      * @return null if the given driver do not park at this node, otherwise return itself
      */
	const Agent* leaveFrom(const Node* node, const Agent* agent);

    /**
      * remove a driver from parking coordinator
      * @param id is key to a given agent who will be removed
      * @return null if the given driver do not park at this node, otherwise return itself
      */
	const Agent* remove(const int agentid);

    /**
      * get the capacity at a given node
      * @param node is parking location
      * @return the capacity
      */
	int  getCapacity(const Node* node);

    /**
      * get the occupancy at a given node
      * @param node is parking location
      * @return the occupancy currently
      */
	int  getOccupancy(const Node* node);

    /**
      * check whether a driver already park at a give node
      * @param node is parking location
      * @param agent is the driver who will be checked whether or not parking at this node
      * @return the occupancy currently
      */
	bool isExisted(const Node* node, const Agent* agent);

private:
	std::map<const Node*, ParkingLot > vehicleParking;

};

}

} /* namespace sim_mob */
#endif /* PARKINGCOORDINATOR_H_ */
