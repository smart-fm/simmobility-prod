//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/Person.hpp"

namespace sim_mob
{

class Person_MT : public Person
{
private:
	/**Current lane (used by confluxes to track the person)*/
	const sim_mob::Lane* currLane;

	/***/
	const sim_mob::SegmentStats* currSegStats;

	/***/
	const sim_mob::Link* nextLinkRequired;

	/**Alters trip chain in accordance to route choice for public transit trips*/
	void convertODsToTrips();

	/**Inserts a waiting activity before bus travel*/
	void insertWaitingActivityToTrip();

	/**Assigns id to sub-trips*/
	void assignSubtripIds();

protected:
public:
	/**Indicates if the agent is queuing*/
	bool isQueuing;

	/**The distance to the end of the segment*/
	double distanceToEndOfSegment;

	/**The time taken to drive to the end of the link*/
	double drivingTimeToEndOfLink;

	//Used by confluxes and movement facet of roles to move this person in the medium term
	const sim_mob::SegmentStats* requestedNextSegStats;

	enum Permission //to be renamed later
	{
		NONE = 0,
		GRANTED,
		DENIED
	};
	Permission canMoveToNextSegment;

	Person_MT(const std::string& src, const MutexStrategy& mtxStrat, int id = -1, std::string databaseID = "");
	Person_MT(const std::string& src, const MutexStrategy& mtxStrat, const std::vector<sim_mob::TripChainItem*>& tc);
	virtual ~Person_MT();

	/**
	 * Initialises the trip chain
     */
	void initTripChain() = 0;

	const sim_mob::Lane* getCurrLane() const
	{
		return currLane;
	}

	void setCurrLane(const sim_mob::Lane* currLane)
	{
		this->currLane = currLane;
	}

	const sim_mob::SegmentStats* getCurrSegStats() const
	{
		return currSegStats;
	}

	void setCurrSegStats(const sim_mob::SegmentStats* currSegStats)
	{
		this->currSegStats = currSegStats;
	}

	const Link* getNextLinkRequired() const
	{
		return nextLinkRequired;
	}

	void setNextLinkRequired(Link* nextLink)
	{
		nextLinkRequired = nextLink;
	}
};
}