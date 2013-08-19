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

class ParkingCoordinator {
public:
	ParkingCoordinator();
	virtual ~ParkingCoordinator();

	struct ParkingLot{
		ParkingLot() : node(0), maxCapacity(10), currentOccupancy(0) {;}
		const Node* node;
		int maxCapacity;
		int currentOccupancy;
		std::vector<const Agent*> vehicles;
	};

	bool enterTo(const Node* node, const Agent* agent);
	const Agent* leaveFrom(const Node* node, const Agent* agent);
	const Agent* remove(const int agentid);
	int  getCapacity(const Node* node);
	int  getOccupancy(const Node* node);
	bool isExisted(const Node* node, const Agent* agent);

private:
	std::map<const Node*, ParkingLot > vehicle_parking;

};

}

} /* namespace sim_mob */
#endif /* PARKINGCOORDINATOR_H_ */
