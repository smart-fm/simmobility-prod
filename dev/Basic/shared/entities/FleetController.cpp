/*
 * TaxiFleetManager.cpp
 *
 *  Created on: Jan 11, 2017
 *      Author: zhang huai peng
 */

#include <boost/date_time.hpp>

#include "FleetController.hpp"
#include "conf/ConfigManager.hpp"
#include "util/Utils.hpp"
#include "workers/Worker.hpp"

namespace bt = boost::posix_time;
using namespace sim_mob;

double getSecondFrmTimeString(const std::string& startTime)
{
	std::istringstream is(startTime);
	is.imbue(std::locale(is.getloc(),new bt::time_input_facet("%d-%m-%Y %H:%M")));
	bt::ptime pt;
	is >> pt;
	return (double)pt.time_of_day().ticks() / (double)bt::time_duration::rep_type::ticks_per_second;
}

void FleetController::LoadTaxiFleetFromDB()
{
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	soci::session sql_(soci::postgresql,cfg.getDatabaseConnectionString(false));
	const std::map<std::string, std::string>& storedProcs = cfg.getDatabaseProcMappings().procedureMappings;
	std::map<std::string, std::string>::const_iterator spIt = storedProcs.find("taxi_fleet");

	if (spIt == storedProcs.end())
	{
		return;
	}

	std::map<std::string, std::vector<TripChainItem> > tripchains;
	soci::rowset<soci::row> rs = (sql_.prepare<< "select * from " + spIt->second);

	for (soci::rowset<soci::row>::const_iterator it = rs.begin();it != rs.end(); ++it)
	{
		FleetItem fleetItem;
		const soci::row& r = (*it);
		fleetItem.vehicleNo = r.get<std::string>(0);
		fleetItem.driverId = r.get<std::string>(1);
		double x = r.get<double>(2);
		double y = r.get<double>(3);
		Utils::convertWGS84_ToUTM(x, y);
		fleetItem.startNode = Node::allNodesMap.searchNearestObject(x, y);
		std::string startTime = r.get<std::string>(4);
		fleetItem.startTime = getSecondFrmTimeString(startTime);
		fleetItem.controllerSubscription = r.get<unsigned int>(5);
		taxiFleet.push_back(fleetItem);
	}
}

std::vector<FleetController::FleetItem> FleetController::dispatchTaxiAtCurrentTime(const unsigned int currentTimeSec)
{
	std::vector<FleetItem> res;
	auto i = taxiFleet.begin();
	while (i != taxiFleet.end())
	{
		if (i->startTime <= currentTimeSec)
		{
			res.push_back(*i);
			i = taxiFleet.erase(i);
		}
		else
		{
			i++;
		}
	}
	return res;
}

const std::vector<FleetController::FleetItem>& FleetController::getTaxiFleet() const
{
	return taxiFleet;
}

Entity::UpdateStatus FleetController::frame_init(timeslice now)
{
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus FleetController::frame_tick(timeslice now)
{
	nextTimeTickToStage++;
	unsigned int nextTickMS = nextTimeTickToStage * ConfigManager::GetInstance().FullConfig().baseGranMS();

	//Stage any pending entities that will start during this time tick.
	while (!pendingChildren.empty() && pendingChildren.top()->getStartTime() <= nextTickMS)
	{
		//Ask the current worker's parent WorkGroup to schedule this Entity.
		Entity* child = pendingChildren.top();
		pendingChildren.pop();
		child->parentEntity = this;
		currWorkerProvider->scheduleForBred(child);
	}

	return Entity::UpdateStatus::Continue;
}