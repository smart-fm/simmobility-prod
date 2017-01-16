/*
 * TaxiFleetManager.cpp
 *
 *  Created on: Jan 11, 2017
 *      Author: zhang huai peng
 */

#include "TaxiFleetManager.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include <soci/postgresql/soci-postgresql.h>
#include <soci/soci.h>
#include "util/Utils.hpp"
#include <iostream>
#include <boost/date_time.hpp>

namespace bt = boost::posix_time;
namespace sim_mob
{
TaxiFleetManager* TaxiFleetManager::instance = nullptr;

std::time_t getSecondFrmTime(const bt::ptime& pt)
{
	std::cout<<pt.time_of_day().hours()<<","<<pt.time_of_day().minutes()<<std::endl;
	return pt.time_of_day().ticks() / bt::time_duration::rep_type::ticks_per_second;
}

TaxiFleetManager::TaxiFleetManager()
{
	LoadTaxiDemandFrmDB();
}

TaxiFleetManager* TaxiFleetManager::getInstance()
{
	if(!instance){
		instance = new TaxiFleetManager();
	}
	return instance;
}

TaxiFleetManager::~TaxiFleetManager()
{

}

void TaxiFleetManager::LoadTaxiDemandFrmDB()
{
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	soci::session sql_(soci::postgresql,cfg.getDatabaseConnectionString(false));
	const std::map<std::string, std::string>& storedProcs =
			cfg.getDatabaseProcMappings().procedureMappings;
	std::map<std::string, std::string>::const_iterator spIt = storedProcs.find("taxi_fleet");
	if (spIt == storedProcs.end())
	{
		return;
	}

	std::map<std::string, std::vector<TripChainItem> > tripchains;
	soci::rowset<soci::row> rs = (sql_.prepare<< "select * from " + spIt->second);
	for (soci::rowset<soci::row>::const_iterator it = rs.begin();it != rs.end(); ++it)
	{
		TaxiFleet taxiFleet;
		const soci::row& r = (*it);
		taxiFleet.vehicleNo = r.get<std::string>(0);
		taxiFleet.driverId = r.get<std::string>(1);
		double x = r.get<double>(2);
		double y = r.get<double>(3);
		Utils::convertWGS84_ToUTM(x, y);
		taxiFleet.startNode = Node::allNodesMap.searchNearestObject(x, y);
		bt::ptime pt;
		std::string startTime = r.get<std::string>(4);
		std::istringstream is(startTime);
		is.imbue(std::locale(is.getloc(),new bt::time_input_facet("%d-%m-%Y %H:%M")));
		is >> pt;
		taxiFleet.startTime = getSecondFrmTime(pt);
		taxiFleets.push_back(taxiFleet);
	}
}

std::vector<TaxiFleetManager::TaxiFleet> TaxiFleetManager::dispatchTaxiAtCurrentTime(const unsigned int current)
{
	std::vector<TaxiFleet> res;
	for(auto i=taxiFleets.begin(); i!=taxiFleets.end(); i++)
	{
		if(i->startTime > current)
		{
			res.push_back(*i);
		}
	}
	return res;
}
}
