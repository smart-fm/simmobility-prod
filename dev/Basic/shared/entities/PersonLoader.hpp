//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/noncopyable.hpp>
#include <set>
#include <string>
#include <soci/soci.h>
#include <soci/postgresql/soci-postgresql.h>

namespace sim_mob
{
class Entity;
class Person;
class StartTimePriorityQueue;

/**
 * Singleton to periodically load person demand
 *
 * \author Harish Loganathan
 */
class PeriodicPersonLoader :  private boost::noncopyable
{
protected:
	/** our active agents list*/
	std::set<sim_mob::Entity*>& activeAgents;

	/** out pending agents list*/
	StartTimePriorityQueue& pendingAgents;

	/** data load interval in seconds*/
	unsigned dataLoadInterval;

	/** start time of next load interval*/
	double nextLoadStart;

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
	 * load person demand for next interval
	 */
	virtual void loadPersonDemand() = 0;

	/**
	 * tracks the number of ticks elapsed since last load
	 * this function *must* be called in every tick
	 * @return true if data has to be loaded in this tick; false otherwise
	 */
	bool checkTimeForNextLoad();
};
}






