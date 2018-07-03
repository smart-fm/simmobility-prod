/*
 * TravellerStatsManager.cpp
 *
 *  Created on: Nov 3, 2016
 *      Author: fm-simmobility
 */

#include "TravellerStatsManager.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

namespace sim_mob
{
TravellerStatsManager* TravellerStatsManager::instance = nullptr;

TravellerStatsManager* TravellerStatsManager::getInstance()
{
	if(!instance)
	{
		instance = new TravellerStatsManager();
	}
	return instance;
}

TravellerStatsManager::TravellerStatsManager() {

}

TravellerStatsManager::~TravellerStatsManager() {

}

void TravellerStatsManager::loadHistoricalData()
{
	const sim_mob::ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();
	std::string dbStr(cfg.getDatabaseConnectionString(false));
	soci::session dbSession(soci::postgresql, dbStr);
	std::string query =	"SELECT waitingnum, time_index, link_id  FROM supply.waitingtaxi_atlink;";
	soci::rowset<soci::row> rs = (dbSession.prepare << query);
	for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); ++it) {
		const soci::row& rowData = *it;
		unsigned int waitingNum = rowData.get<unsigned int>(0);
		unsigned int timeIndex = rowData.get<unsigned int>(1);
		unsigned int linkId = rowData.get<unsigned int>(2);
		std::pair<unsigned int, unsigned int> index = std::make_pair(timeIndex, linkId);
		historicalData[index] = waitingNum;
	}
}

int TravellerStatsManager::getWaitingNumber(unsigned int linkId, unsigned int currentTimeSec)
{
	int res = 0;
	unsigned int timeIndex = currentTimeSec / timeIntervalSec;
	std::pair<unsigned int, unsigned int> index = std::make_pair(timeIndex, linkId);
	auto it = historicalData.find(index);
	if (it != historicalData.end()) {
		res = it->second;
	}
	return res;
}

}

