//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PersonLoader.hpp"

#include <boost/lexical_cast.hpp>
#include <map>
#include <sstream>
#include <stdint.h>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/misc/TripChain.hpp"
#include "util/DailyTime.hpp"
#include "util/Utils.hpp"

using namespace sim_mob;

namespace
{
	const double DEFAULT_LOAD_INTERVAL = 0.5; // for convenience. see loadActivitySchedule function

	/**
	 * given a time value in seconds measured from 00:00:00 (12AM)
	 * this function returns a numeric representation of the half hour window of the day
	 * the time belongs to.
	 * The 48 numeric representations of the day are as follows
	 * 3.25 = (3:00 - 3:29)
	 * 3.75 = (3:30 - 3:59)
	 * 4.25 = (4:00 - 4:29)
	 * 4.75 = (4:30 - 4:59)
	 * .
	 * . and so on
	 * .
	 * 23.75 = (23:30 - 23:59)
	 * 24:25 = (0:00 - 0:29) of next day
	 * 24.75 = (0:30 - 0:59)
	 * 25.25 = (1:00 - 1:29)
	 * .
	 * . and so on
	 * .
	 * 26.75 = (2:30 - 2:59)
	 */
	double getHalfHourWindow(uint32_t time) //time is in seconds
	{
		uint32_t hour = time/3600;
		uint32_t remainder = time % 3600;
		uint32_t minutes = remainder/60;
		if(hour < 3) { hour = hour+24; }
		if(minutes < 30) { return (hour + 0.25); }
		else { return (hour + 0.75); }
	}

	/**
	 * generates a random time within the time window passed in preday's representation.
	 *
	 * @param mid time window in preday format (E.g. 4.75 => 4:30 to 4:59 AM)
	 * @return a random time within the window in hh24:mm:ss format
	 */
	std::string getRandomTimeInWindow(double mid) {
		int hour = int(std::floor(mid));
		int minute = (Utils::generateInt(0,29)) + ((mid - hour - 0.25)*60);
		std::stringstream random_time;
		hour = hour % 24;
		if (hour < 10) {
			random_time << "0" << hour << ":";
		}
		else {
			random_time << hour << ":";
		}
		if (minute < 10) {
			random_time << "0" << minute << ":";
		}
		else {
			random_time << minute << ":";
		}
		random_time << "00"; //seconds
		return random_time.str();
	}

	sim_mob::Activity* makeActivity(const soci::row& r, unsigned int seqNo)
	{
		sim_mob::RoadNetwork& rn = ConfigManager::GetInstanceRW().FullConfig().getNetworkRW();
		sim_mob::Activity* res = new sim_mob::Activity();
		res->setPersonID(r.get<string>(0));
		res->itemType = sim_mob::TripChainItem::IT_ACTIVITY;
		res->sequenceNumber = seqNo;
		res->description = r.get<string>(4);
		res->isPrimary = r.get<int>(7);
		res->isFlexible = false;
		res->isMandatory = true;
		res->location = rn.getNodeById(r.get<int>(5));
		res->locationType = sim_mob::TripChainItem::LT_NODE;
		res->startTime = sim_mob::DailyTime(getRandomTimeInWindow(r.get<double>(8)));
		res->endTime = sim_mob::DailyTime(getRandomTimeInWindow(r.get<double>(9)));
		return res;
	}

	sim_mob::Trip* makeTrip(const soci::row& r, unsigned int seqNo, unsigned short tripNo)
	{
		sim_mob::RoadNetwork& rn = ConfigManager::GetInstanceRW().FullConfig().getNetworkRW();
		sim_mob::ConfigParams& config = sim_mob::ConfigManager::GetInstanceRW().FullConfig();
		sim_mob::Trip* tripToSave = new sim_mob::Trip();
		tripToSave->tripID = boost::lexical_cast<string>(tripNo);
		tripToSave->setPersonID(r.get<string>(0));
		tripToSave->itemType = sim_mob::TripChainItem::IT_TRIP;
		tripToSave->sequenceNumber = seqNo;
		tripToSave->fromLocation = sim_mob::WayPoint(rn.getNodeById(r.get<int>(10)));
		tripToSave->fromLocationType = sim_mob::TripChainItem::LT_NODE;
		tripToSave->toLocation = sim_mob::WayPoint(rn.getNodeById(r.get<int>(5)));
		tripToSave->toLocationType = sim_mob::TripChainItem::LT_NODE;
		tripToSave->startTime = sim_mob::DailyTime(getRandomTimeInWindow(r.get<double>(11)));
		return tripToSave;
	}

	sim_mob::SubTrip makeSubTrip(const soci::row& r, sim_mob::Trip* parentTrip, unsigned short stopNo=1)
	{
		sim_mob::RoadNetwork& rn = ConfigManager::GetInstanceRW().FullConfig().getNetworkRW();
		sim_mob::ConfigParams& config = sim_mob::ConfigManager::GetInstanceRW().FullConfig();
		sim_mob::SubTrip aSubTripInTrip;
		aSubTripInTrip.setPersonID(r.get<string>(0));
		aSubTripInTrip.itemType = sim_mob::TripChainItem::IT_TRIP;
		aSubTripInTrip.tripID = parentTrip->tripID + "-" + boost::lexical_cast<string>(stopNo);
		aSubTripInTrip.fromLocation = sim_mob::WayPoint(rn.getNodeById(r.get<int>(10)));
		aSubTripInTrip.fromLocationType = sim_mob::TripChainItem::LT_NODE;
		aSubTripInTrip.toLocation = sim_mob::WayPoint(rn.getNodeById(r.get<int>(5)));
		aSubTripInTrip.toLocationType = sim_mob::TripChainItem::LT_NODE;
		aSubTripInTrip.mode = r.get<string>(6);
		aSubTripInTrip.isPrimaryMode = r.get<int>(7);
		aSubTripInTrip.startTime = parentTrip->startTime;
		return aSubTripInTrip;
	}
}

sim_mob::PeriodicPersonLoader::PeriodicPersonLoader(std::set<sim_mob::Entity*>& activeAgents, StartTimePriorityQueue& pendinAgents)
	: activeAgents(activeAgents), pendinAgents(pendinAgents),
	  sql_(soci::postgresql, ConfigManager::GetInstanceRW().FullConfig().getDatabaseConnectionString(false))
{
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	dataLoadInterval = DEFAULT_LOAD_INTERVAL; //cfg.system.genericProps.at("activity_load_interval"); //TODO read from config
	nextLoadStart = getHalfHourWindow(cfg.system.simulation.simStartTime.getValue()/1000);
	storedProcName = cfg.getDatabaseProcMappings().procedureMappings["day_activity_schedule"];
}

sim_mob::PeriodicPersonLoader::~PeriodicPersonLoader() {}

void sim_mob::PeriodicPersonLoader::loadActivitySchedules()
{
	if (storedProcName.empty()) { return; }
	//Our SQL statement
	stringstream query;
	unsigned end = nextLoadStart + dataLoadInterval;
	query << "select * from " << storedProcName << "(" << nextLoadStart << "," << end << ")";
	std::string sql_str = query.str();
	soci::rowset<soci::row> rs = (sql_.prepare << sql_str);
	for (soci::rowset<soci::row>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
	{

	}
	nextLoadStart = end;																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																	nextLoadStart = end; //update for next loading
}
