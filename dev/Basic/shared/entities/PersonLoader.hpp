//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>
#include <set>
#include <string>

#include "soci.h"
#include "soci-postgresql.h"

namespace sim_mob
{
class Entity;
class Agent;
class Person;
class StartTimePriorityQueue;

class PeriodicPersonLoader :  private boost::noncopyable
{
private:
	/** map of <person_id, Person*> for persons loaded so far in the simulation*/
	boost::unordered_map<std::string, Person*> loadedPersons;

	/** our active agents list*/
	std::set<sim_mob::Entity*>& activeAgents;

	/** out pending agents list*/
	StartTimePriorityQueue& pendingAgents;

	/** data load interval in seconds*/
	unsigned dataLoadInterval;

	/** start time of next load interval*/
	double nextLoadStart;

	/** database session */
	soci::session sql_;

	/** stored procedure to periodically load data*/
	std::string storedProcName;

	/** time elapsed since previous load in seconds*/
	unsigned elapsedTimeSinceLastLoad;

	/**
	 * adds person to active or pending agents list depending on start time
	 */
	void addOrStashPerson(Person* p);

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

	const std::string& getStoredProcName() const
	{
		return storedProcName;
	}

	/**
	 * load activity schedules for next interval
	 */
	void loadActivitySchedules();

	/**
	 * tracks the number of ticks elapsed since last load
	 * this function *must* be called in every tick
	 * @return true if data has to be loaded in this tick; false otherwise
	 */
	bool checkTimeForNextLoad();
};
}






