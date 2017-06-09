/*
 * TaxiFleetManager.hpp
 *
 *  Created on: Jan 11, 2017
 *      Author: zhang huai peng
 */

#ifndef TAXIFLEETMANAGER_HPP_
#define TAXIFLEETMANAGER_HPP_

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
		double endTime= 0;
		const Node* startNode = nullptr;
		unsigned int controllerSubscription = 0;
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
	 * Dispatches a taxi at the given current time
	 * @param currentTimeSec is current time in seconds
	 * @return a list include all taxi which need be dispatched currently
	 */
	std::vector<FleetItem> dispatchTaxiAtCurrentTime(const unsigned int currentTimeSec);

	/**
	 * Returns the vector of fleet items (taxis)
	 */
	const std::vector<FleetItem>& getTaxiFleet() const;

protected:
	/**Stores taxi information*/
	std::vector<FleetItem> taxiFleet;

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
#endif /* TAXIFLEETMANAGER_HPP_ */
