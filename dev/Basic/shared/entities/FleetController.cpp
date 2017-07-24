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

namespace
{
	enum TaxiFleetTableColumns
	{
		COLUMN_VEHICLE_NUMBER = 0,
		COLUMN_DRIVER_ID = 1,
		COLUMN_START_LOCATION_X = 2,
		COLUMN_START_LOCATION_Y = 3,
		COLUMN_SHIFT_START_TIME = 4,
		COLUMN_CONTROLLER_SUBSCRIPTIONS = 5,
		COLUMN_SHIFT_DURATION = 6
	};
}

double getSecondFrmTimeString(const std::string& startTime)
{
	std::istringstream is(startTime);
	is.imbue(std::locale(is.getloc(),new bt::time_input_facet("%H:%M:%S")));
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
	std::stringstream query;

	const SimulationParams &simParams = ConfigManager::GetInstance().FullConfig().simulation;

	query << "select * from " << spIt->second << "('" << simParams.simStartTime.getStrRepr().substr(0, 5)
	      << "','" << (DailyTime(simParams.totalRuntimeMS) + simParams.simStartTime).getStrRepr().substr(0, 5) << "')";
	soci::rowset<soci::row> rs = (sql_.prepare << query.str());

	for (soci::rowset<soci::row>::const_iterator it = rs.begin();it != rs.end(); ++it)
	{
		FleetItem fleetItem;
		const soci::row& r = (*it);
		fleetItem.vehicleNo = r.get<std::string>(COLUMN_VEHICLE_NUMBER);
		fleetItem.driverId = r.get<std::string>(COLUMN_DRIVER_ID);
		double x = r.get<double>(COLUMN_START_LOCATION_X);
		double y = r.get<double>(COLUMN_START_LOCATION_Y);
		Utils::convertWGS84_ToUTM(x, y);
		fleetItem.startNode = Node::allNodesMap.searchNearestObject(x, y);
		const std::string &startTime = r.get<std::string>(COLUMN_SHIFT_START_TIME);
		fleetItem.startTime = getSecondFrmTimeString(startTime);
		int shiftDuration = 3600 * r.get<int>(COLUMN_SHIFT_DURATION);
		fleetItem.endTime = fleetItem.startTime + shiftDuration;
		fleetItem.controllerSubscription = r.get<unsigned int>(COLUMN_CONTROLLER_SUBSCRIPTIONS);
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