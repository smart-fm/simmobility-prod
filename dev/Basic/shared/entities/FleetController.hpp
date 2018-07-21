//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <vector>
#include <queue>

#include "entities/Agent.hpp"

namespace sim_mob
{
class Node;

class FleetController : public Agent
{
private:
	/**Records next time tick to help dispatching decision*/
	uint32_t nextTimeTickToStage;

public:
	explicit FleetController(int id = -1, const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered) : Agent(mtxStrat, id),
	nextTimeTickToStage(0)
	{
		startTime = 0;
		LoadTaxiFleetFromDB();
	}

	virtual ~FleetController()
	{
	}

	struct FleetItem
	{
		std::string vehicleNo;
		std::string driverId;
		double startTime = 0;
		double endTime = 0;
        int segmentId = 0;
        int vehicleType = 0;
		const Node* startNode = nullptr;
		int controllerSubscription = 0;
        int passengerCapacity = 0;
        int pcu=0;
        int av = 0;
	};

	struct cmp_fleet_start: public std::less<FleetItem>
	{
		bool operator()(const FleetItem& x, const FleetItem& y) const
		{
			return x.startTime > y.startTime;
		}
	};

	class FleetTimePriorityQueue: public std::priority_queue<FleetItem,
			std::vector<FleetItem>, cmp_fleet_start>
	{
	};

	/**
	 * Virtual method for initialisation of the fleet manager
	 * @param agentList
	 */
	virtual void initialise(std::set<sim_mob::Entity*>& agentList) = 0;

	/**
	 * Returns the vector of fleet items (taxis)
	 */

protected:
	/**Stores taxi information*/;
	std::multimap<unsigned int,FleetItem> taxiFleet;
	std::multimap<unsigned int,FleetItem>::const_iterator taxiFleetIt;

	/**
	 * Taxis waiting to be added to the simulation, prioritized by start time.
	 */
	StartTimePriorityQueue pendingChildren;

	/**
	 * Load taxi fleet from database
	 */
	void LoadTaxiFleetFromDB();

	/**
	 * Inherited from base class agent to initialize parameters for fleet manager
	 * @return update status
	 */
	virtual Entity::UpdateStatus frame_init(timeslice);

	/**
	 * Inherited from base class to update this agent
	 * @return update status
	 */
	virtual Entity::UpdateStatus frame_tick(timeslice);

	/**
	 * Inherited from base class to output result
	 */
	virtual void frame_output(timeslice)
	{
	}

	/**
	 * Returns true if the entity is non-spatial
	 */
	virtual bool isNonspatial()
	{
		return true;
	}
};
}

