//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PersonLoader.hpp"

#include <sstream>
#include <stdint.h>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

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
}

sim_mob::PeriodicPersonLoader::PeriodicPersonLoader(std::set<sim_mob::Entity*>& activeAgents, StartTimePriorityQueue& pendinAgents)
	: activeAgents(activeAgents), pendinAgents(pendinAgents), sql_(soci::postgresql, ConfigManager::GetInstanceRW().FullConfig().getDatabaseConnectionString(false))
{
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	dataLoadInterval = DEFAULT_LOAD_INTERVAL; //cfg.system.genericProps.at("activity_load_interval"); //TODO read from config
	nextLoadStart = getHalfHourWindow(cfg.system.simulation.simStartTime.getValue()/1000);
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

	nextLoadStart = end;																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																	nextLoadStart = end; //update for next loading
}
