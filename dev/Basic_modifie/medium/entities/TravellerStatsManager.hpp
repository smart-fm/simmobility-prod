/*
 * TravellerStatsManager.hpp
 *
 *  Created on: Nov 3, 2016
 *      Author: fm-simmobility
 */

#ifndef TRAVELLERSTATSMANAGER_HPP_
#define TRAVELLERSTATSMANAGER_HPP_

#include "geospatial/network/Link.hpp"

namespace sim_mob
{
class TravellerStatsManager {
public:
	TravellerStatsManager();
	virtual ~TravellerStatsManager();

public:
	/**
	 * gets the singleton instance of TravellerStatsManager
	 */
	static sim_mob::TravellerStatsManager* getInstance();
	/**
	 * load historical data from database
	 */
	void loadHistoricalData();
	/**
	 * query historical data from a given link
	 * @param linkId is id of the given link
	 * @param currentTimeSec indicate current time
	 * @return waiting person number for taxi
	 */
	int getWaitingNumber(unsigned int linkId, unsigned int currentTimeSec);
private:
	/**record current instance to this object*/
	static TravellerStatsManager* instance;
	/**define time interval in seconds*/
	static const unsigned int timeIntervalSec = 600;
	/**store historical data*/
	std::map<std::pair<unsigned int, unsigned int>,unsigned int> historicalData;
};

}
#endif /* TRAVELLERSTATSMANAGER_HPP_ */
