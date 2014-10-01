//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/unordered_map.hpp>
#include <set>
#include <string>

#include "Agent.hpp"
#include "Person.hpp"
#include "soci.h"
#include "soci-postgresql.h"


using namespace std;
using namespace sim_mob;

namespace sim_mob
{
class PeriodicPersonLoader
{
private:
	/** map of <person_id, Person*> for persons loaded so far in the simulation*/
	boost::unordered_map<string, Person*> loadedPersons;

	/** our active agents list*/
	std::set<sim_mob::Entity*>& activeAgents;

	/** out pending agents list*/
	StartTimePriorityQueue& pendinAgents;

	/** data load interval in seconds*/
	unsigned dataLoadInterval;

	/** start time of next load interval*/
	unsigned nextLoadStart;

	/** database session */
	soci::session sql_;

	/** stored procedure to periodically load data*/
	string storedProcName;

public:
	PeriodicPersonLoader(std::set<sim_mob::Entity*>& activeAgents, StartTimePriorityQueue& pendinAgents);
	virtual ~PeriodicPersonLoader();

	unsigned getDataLoadInterval() const
	{
		return dataLoadInterval;
	}

	unsigned getNextLoadStart() const
	{
		return nextLoadStart;
	}

	const string& getStoredProcName() const
	{
		return storedProcName;
	}

	/**
	 * load activity schedules for next interval
	 */
	void loadActivitySchedules();
};
}






