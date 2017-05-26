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
namespace sim_mob
{
class Node;

class FleetManager
{
public:
	FleetManager();
	virtual ~FleetManager();

	struct FleetItem
	{
		std::string vehicleNo;
		std::string driverId;
		double startTime = 0;
		double endTime= 0;
		const Node* startNode = nullptr;
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
	 * load taxi demand from database
	 */
	void LoadTaxiFleetFromDB();

	/**
	 * get global singleton instance of FleetManager
	 * @return pointer to the global instance of FleetManager
	 */
	static FleetManager* getInstance();

	/**
	 * get taxi fleet at current time
	 * @param currentTimeSec is current time in seconds
	 * @return a list include all taxi which need be dispatched currently
	 */
	std::vector<FleetItem> dispatchTaxiAtCurrentTime(const unsigned int currentTimeSec);

	/**
	 * get all taxi fleet information
	 */
	const std::vector<FleetItem>& getAllTaxiFleet() const;

private:
	/**store taxi fleet information*/
	std::vector<FleetItem> taxiFleet;

	/**store self instance*/
	static FleetManager* instance;
};
}
#endif /* TAXIFLEETMANAGER_HPP_ */
