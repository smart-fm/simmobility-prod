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

	/**The previous role that was played by the person.*/
	Role<Person_MT>* prevRole;

	/**The current role being played by the person*/
	Role<Person_MT>* currRole;

	/**The next role that the person will play. However, this variable is only temporary and will not be used to update the currRole*/
	Role<Person_MT>* nextRole;

	/**Alters trip chain in accordance to route choice for public transit trips*/
	void convertODsToTrips();

	/**Inserts a waiting activity before bus travel*/
	void insertWaitingActivityToTrip();

	/**Assigns id to sub-trips*/
	void assignSubtripIds();

	/**
	 * Advances the current trip chain item to the next item if all the sub-trips in the trip have been completed.
	 * If not, calls the advanceCurrentTripChainItem() method
     *
	 * @return true, if the trip chain item is advanced
     */
	bool advanceCurrentTripChainItem();

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
	void initTripChain();

	/**
	 * Check if any role changing is required.

	 * @return
     */
	Entity::UpdateStatus checkTripChain();

	/**
	 * Sets the simulation start time of the entity
	 *
     * @param value The simulation start time to be set
     */
	virtual void setStartTime(unsigned int value);

	/**
	 * Updates the person's current role to the given role.
	 *
     * @param newRole Indicates the new role of the person. If the new role is not provided,
	 * a new role is created based on the current sub-trip
	 *
     * @return true, if role is updated successfully
     */
	bool updatePersonRole();

	/**
	 * Change the role of this person
	 *
     * @param newRole the new role to be assigned to the person
     */
	void changeRole();

	/**
	 * Builds a subscriptions list to be added to the managed data of the parent worker
	 *
	 * @return the list of Buffered<> types this entity subscribes to
	 */
	virtual std::vector<BufferedBase *> buildSubscriptionList() = 0;

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

	Role<Person_MT>* getRole() const
	{
		return currRole;
	}

	Role<Person_MT>* getPrevRole() const
	{
		return prevRole;
	}

	void setNextRole(Role<Person_MT> *newRole)
	{
		nextRole = newRole;
	}

	Role<Person_MT>* getNextRole() const
	{
		return nextRole;
	}
};
}