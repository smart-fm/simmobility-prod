//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/noncopyable.hpp>
#include <set>
#include <string>

#include "soci.h"
#include "soci-postgresql.h"

#include "entities/misc/TripChain.hpp"
#include "util/OneTimeFlag.hpp"

namespace sim_mob
{
class Entity;
class Agent;
class Person;
class StartTimePriorityQueue;
//FDs for RestrictedRegion
class Node;
class TripChainItem;
class WayPoint;


class CBD_Pair
{
public:
	int from_section,to_section;//to avoid confusion, code style conforms to database
};

class RestrictedRegion : private boost::noncopyable
{
	/*
	 * all segments of the restricted area(aka CBD)
	 */
	std::set<const sim_mob::RoadSegment*> zoneSegments;//get_ban_section_CBD_aimsun()
	/**
	 * restricted area border segments
	 * categorized based on in segments and out segments
	 * each tem in each container is a from/to segment pair:
	 * for example in 'in' container, the first item in each pair
	 * demonstrates the segment before getting into the restricted
	 * area, and the second item is the first segment inside the
	 * restricted area.
	 * similarly, in 'out' container, the first item in each pair
	 * demonstrates the segment before getting out of the restricted
	 * area, and the second item is the first segment outside the
	 * restricted area.
	 */
	typedef std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> SegPair;
	std::set<SegPair> in, out;
	std::set<const Node*> zoneNodes;
	sim_mob::OneTimeFlag populated;
public:
	/**
	 * does the given "node"(or node wrapped in a WayPoint) lie in the restricted area,
	 * returns the periphery node if the target is in the restricted zone
	 * returns null if Node not found in the restricted region.
	 */
	 const Node* isInRestrictedZone(const Node* target) const;
	 const Node* isInRestrictedZone(const WayPoint& target) const;
	/**
	 * does the given "RoadSegment"(segment wrapped in a WayPoint) lie in the restricted area,
	 * returns true if the target is in the restricted zone
	 */
	 bool isInRestrictedZone(const sim_mob::RoadSegment * target) const;
	 bool isEnteringRestrictedZone(const sim_mob::RoadSegment* curSeg ,const sim_mob::RoadSegment* nxtSeg);
	 bool isExittingRestrictedZone(const sim_mob::RoadSegment* curSeg ,const sim_mob::RoadSegment* nxtSeg);
	 std::set<const sim_mob::RoadSegment*> getZoneSegments() { return zoneSegments;}
	/**
	 * Function to split the subtrips crossing the restricted Areas
	 */
	void processSubTrips(std::vector<sim_mob::SubTrip>& subTrips);
	/**
	 * fill the input data into in,out,zoneSegments
	 * generate data based on input for zoneNodes
	 */
	void populate();
	static boost::shared_ptr<RestrictedRegion> instance;
	/**
	 * returns the singletone instance
	 */
	static RestrictedRegion & getInstance()
	{
		if(!instance)
		{
			instance.reset(new RestrictedRegion());
		}
		return *instance;
	}
	/**
	 * modify trips whose orgigin and/or destination lies in a restricted area
	 */
	void processTripChains(std::map<std::string, std::vector<TripChainItem*> > &tripchains);

};

class PeriodicPersonLoader :  private boost::noncopyable
{
private:
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






