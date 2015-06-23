//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <set>
#include <string>

#include "entities/misc/TripChain.hpp"
#include "soci.h"
#include "soci-postgresql.h"
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

/**
 * from_section, to_section pair
 * \author Vahid
 */
class CBD_Pair
{
public:
	int from_section,to_section;//to avoid confusion, code style conforms to database
};

/**
 * Manager class for any restricted region used for case studies
 * \author Vahid
 */
class RestrictedRegion : private boost::noncopyable
{
private:
	/**
	 * set of all segments of the restricted area(aka CBD)
	 */
	std::set<const sim_mob::RoadSegment*> zoneSegments;//get_ban_section_CBD_aimsun()

	/**
	 * restricted area border segments categorized based on in segments and out segments.
	 * each term in each container is a from/to segment pair:
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
	std::map<unsigned int, const Node*> zoneNodes; //<id, node>
	sim_mob::OneTimeFlag populated;

	/**
	 * String representation of the information
	 * used for optimization
	 */
	std::string inStr, outStr;
	std::string zoneNodesStr;
	std::string zoneSegmentsStr;

	class Search
	{
	public:
		Search(RestrictedRegion &instance):instance(instance){}
		virtual bool isInRestrictedZone(const Node* target) const = 0;
		virtual bool isInRestrictedSegmentZone(const sim_mob::RoadSegment * target) const = 0;
	protected:
		RestrictedRegion &instance;
	};

	Search *Impl;

	/**
	 * Instead of searching the objects one by one and comparing IDs,
	 * keep IDs in a string and search the string for an id.
	 */
	class StrSearch : public Search
	{
	public:
		StrSearch(RestrictedRegion &instance);
		bool isInRestrictedZone(const Node* target) const;
		bool isInRestrictedSegmentZone(const sim_mob::RoadSegment * target) const;

	};

	/**
	 * Conventional method
	 */
	class ObjSearch : public Search
	{
	public:
		ObjSearch(RestrictedRegion &instance);
		bool isInRestrictedZone(const Node* target) const;
		bool isInRestrictedSegmentZone(const sim_mob::RoadSegment * target) const;
	};

	/**
	 * Instead of querying for IDs to see if a segment or node is related to CBD,
	 * put a temporary flag inside RoadSegment and Node,
	 * And when the need be, look at the value of that TAG.
	 */
	class TagSearch : public Search
	{
	public:
		TagSearch(RestrictedRegion &instance);
		bool isInRestrictedZone(const Node* target) const;
		bool isInRestrictedSegmentZone(const sim_mob::RoadSegment * target) const;
	};

public:
	RestrictedRegion();
	virtual ~RestrictedRegion();

	/**
	 * does the given "node"(or node wrapped in a WayPoint) lie in the restricted area,
	 * returns the periphery node if the target is in the restricted zone
	 * returns null if Node not found in the restricted region.
	 */
	bool isInRestrictedZone(const Node* target) const;
	bool isInRestrictedZone(const WayPoint& target) const;

	/**
	 * does the given Path "RoadSegments"(segments wrapped in WayPoints) lie in the restricted area,
	 * returns true if any part of the target is in the restricted zone
	 */
	bool isInRestrictedSegmentZone(const std::vector<WayPoint> & target) const;

	/**
	 * does the given Path "RoadSegments" and "Nodes"(segments wrapped in WayPoints) lie in the restricted area,
	 * returns true if any part of the target is in the restricted zone
	 */
	bool isInRestrictedZone(const std::vector<WayPoint>& target) const;

	/**
	 * does the given "RoadSegment" lie in the restricted area,
	 * returns true if the target is in the restricted zone
	 */
	bool isInRestrictedSegmentZone(const sim_mob::RoadSegment * target) const;

	bool isEnteringRestrictedZone(const sim_mob::RoadSegment* curSeg ,const sim_mob::RoadSegment* nxtSeg);
	bool isExitingRestrictedZone(const sim_mob::RoadSegment* curSeg ,const sim_mob::RoadSegment* nxtSeg);

	std::set<const sim_mob::RoadSegment*>& getZoneSegments()
	{
		if(populated.check())
		{
			throw std::runtime_error("Requesting For CBD zone segments before attempting to populate the containers\n");
		}
		return zoneSegments;
	}

	/**
	 * fill the input data into in,out,zoneSegments
	 * generate data based on input for zoneNodes
	 */
	void populate();

	static boost::shared_ptr<RestrictedRegion> instance;

	static RestrictedRegion & getInstance()
	{
		if(!instance)
		{
			instance.reset(new RestrictedRegion());
		}
		return *instance;
	}
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

	/**
	 * makes a single sub trip for trip (for now)
	 * @param r row from database table
	 * @param parentTrip parent Trip for the subtrip to be constructed
	 * @param subTripNo the sub trip number
	 */
	static void makeSubTrip(const soci::row& r, sim_mob::Trip* parentTrip, unsigned short subTripNo=1);

	static sim_mob::Activity* makeActivity(const soci::row& r, unsigned int seqNo);

	static sim_mob::Trip* makeTrip(const soci::row& r, unsigned int seqNo);
};
}






