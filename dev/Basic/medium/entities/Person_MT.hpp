//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <vector>
#include "behavioral/params/PersonParams.hpp"
#include "entities/Person.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/Link.hpp"
#include "entities/roles/Role.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "entities/FleetController.hpp"
#include <boost/algorithm/string.hpp>
namespace sim_mob
{

namespace medium
{
class Conflux;
class SegmentStats;

class Person_MT : public Person
{
private:
	/** Conflux needs access to certain sensitive members of Person_MT */
	friend Conflux;
	/**Current lane (used by confluxes to track the person)*/
	const Lane* currLane;

	/***/
	const SegmentStats* currSegStats;

	/**The previous role that was played by the person.*/
	Role<Person_MT>* prevRole;

	/**The current role being played by the person*/
	Role<Person_MT>* currRole;

	/**The next role that the person will play. However, this variable is only temporary and will not be used to update the currRole*/
	Role<Person_MT>* nextRole;

	/**Used by confluxes to move the person for his tick duration across link and sub-trip boundaries*/
	double remainingTimeThisTick;

	/**	struct containing additional pertinent information about this person */
	PersonParams personInfo;

	Conflux * currentConflux;

	/**store vehicle no*/
	FleetController::FleetTimePriorityQueue taxiFleets;

	/**Alters trip chain in accordance to route choice for public transit trips*/
	void convertPublicTransitODsToTrips(PT_Network& ptNetwork,const std::string& ptPathsetStoredProcName);

	/**Alters trip chain in accordance to route choice for taxi trip*/
	void convertToTaxiTrips();

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

	/**
	 * make new trip from current point
	 * @param stationName is current station name
	 * @param now is current time
	 */
	void EnRouteToNextTrip(const std::string& stationName, const DailyTime& now);

	/**
	 * Inherited from EventListener.
	 * @param eventId
	 * @param ctxId
	 * @param sender
	 * @param args
	 */
	virtual void onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);

	/**Inherited from MessageHandler.*/
	virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message &message);

protected:
public:
	/**Indicates if the agent is queuing*/
	bool isQueuing;

	/**The distance to the end of the segment*/
	double distanceToEndOfSegment;

	/**The time taken to drive to the end of the link*/
	double drivingTimeToEndOfLink;

	//Used by confluxes and movement facet of roles to move this person in the medium term
	const SegmentStats* requestedNextSegStats;
	bool laneUpdated = false;
	const Lane * updatedLane = nullptr;
	const SegmentStats * beforeUpdateSegStat = nullptr;
	double beforeUpdateDistoSegEnd = -1;
	double latestUpdatedFrameTick=-1;
	double beforeUpdateSpeed = -1;

	enum Permission //to be renamed later
	{
		NONE = 0,
		GRANTED,
		DENIED
	};
	Permission canMoveToNextSegment;

	short numTicksStuck;
	const SegmentStats *lastReqSegStats = nullptr;

	Person_MT(const std::string& src, const MutexStrategy& mtxStrat, int id = -1, std::string databaseID = "");
	Person_MT(const std::string& src, const MutexStrategy& mtxStrat, const std::vector<TripChainItem*>& tc);
	virtual ~Person_MT();

	/**
	 * Initialises the trip chain
     */
	virtual void initTripChain();

	/**
	 * Check if any role changing is required.

	 * @return
     */
	Entity::UpdateStatus checkTripChain(unsigned int currentTime=0);

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
	virtual std::vector<BufferedBase *> buildSubscriptionList();

	/**
	 * This interface splits the MRT trips into different trips of different line
	 * after the entire station list is given by rail transit graph
	 * @param railPath is the station list of the train given by rail transit graph
	 * @return is the vector of OD trips created
	 */
	std::vector<sim_mob::OD_Trip> splitMrtTrips(std::vector<std::string> railPath);

	/**
	 * This interface creates creates MRT sub trips for various intermediate trips splitted
	 * @param src is the source of subtrip
	 * @param dest is the destination of subtrip
	 * @return is the OD trip created
	 */
	sim_mob::OD_Trip CreateMRTSubTrips(std::string src,std::string dest);

	/**
	 * This interface finds the MRT trips after route choice and performs RailTransitRoute choice
	 * to give the full route consisting of station names  and then splitting various intermediate
	 * trips and creating subtrips out of them
	 * @param matchedTrips is the trips given after performing route choice.
	 * The result of splitted trips into subtrips is appended in matchedTrips as well
	 * as matchedTrips is passed by reference
	 */
	void  findMrtTripsAndPerformRailTransitRoute(std::vector<sim_mob::OD_Trip>& matchedTrips);

	std::string chooseModeEnRoute(const Trip& trip, unsigned int originNode, const DailyTime& curTime) const;
	/**
	 * exposes the Log function to print in thread local output files
	 */
	void log(std::string line) const;

	const Lane* getCurrLane() const
	{
		return currLane;
	}

	void setCurrLane(const Lane* currLane)
	{
		this->currLane = currLane;
	}

	const SegmentStats* getCurrSegStats() const
	{
		return currSegStats;
	}

	void setCurrSegStats(const SegmentStats* currSegStats)
	{
		this->currSegStats = currSegStats;
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

	double getRemainingTimeThisTick() const
	{
		return remainingTimeThisTick;
	}

	void setRemainingTimeThisTick(double remainingTimeThisTick)
	{
		this->remainingTimeThisTick = remainingTimeThisTick;
	}

	const PersonParams& getPersonInfo() const
	{
		return personInfo;
	}

	void setPersonInfo(const PersonParams& personInfo)
	{
		this->personInfo = personInfo;
	}

	void setTaxiFleet(const FleetController::FleetTimePriorityQueue& fleets)
	{
		taxiFleets = fleets;
	}

	FleetController::FleetTimePriorityQueue& getTaxiFleet()
	{
		return taxiFleets;
	}

	/**
	 * from current role, export service driver
	 * @return service driver if current role support service driver
	 */
	const MobilityServiceDriver* exportServiceDriver() const
	{
		if(currRole)
		{
			return currRole->exportServiceDriver();
		}
		return nullptr;
	}
};
} // namespace medium
} //namespace sim_mob
