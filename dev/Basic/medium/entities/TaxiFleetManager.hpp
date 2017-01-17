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
namespace sim_mob
{
class Node;
class TaxiFleetManager {
public:
	TaxiFleetManager();
	virtual ~TaxiFleetManager();
	struct TaxiFleet
	{
		std::string vehicleNo;
		std::string driverId;
		double startTime;
		const Node* startNode;
	};
public:
	/**
	 * load taxi demand from database
	 */
	void LoadTaxiDemandFrmDB();
	/**
	 * get global singleton instance of FleetManager
	 * @return pointer to the global instance of FleetManager
	 */
	static TaxiFleetManager* getInstance();
	/**
	 * get taxi fleet at current time
	 * @param currentTimeSec is current time in seconds
	 * @return a list include all taxi which need be dispatched currently
	 */
	std::vector<TaxiFleet> dispatchTaxiAtCurrentTime(const unsigned int currentTimeSec);
private:
	/**store taxi fleet information*/
	std::vector<TaxiFleet> taxiFleets;
	/**store self instance*/
	static TaxiFleetManager* instance;
};
}
#endif /* TAXIFLEETMANAGER_HPP_ */
