//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/foreach.hpp>
#include <map>
#include <string>
#include <vector>

#include "conf/settings/DisableMPI.h"
#include "entities/Agent.hpp"
#include "entities/amodController/AMODEvent.hpp"
#include "entities/conflux/Conflux.hpp"
#include "entities/conflux/SegmentStats.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/vehicle/VehicleBase.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "util/LangHelpers.hpp"
#include "util/Profiler.hpp"
#include "workers/Worker.hpp"

namespace sim_mob
{

class Role;
class TripChainItem;
class SubTrip;
class PartitionManager;
class PackageUtils;
class UnPackageUtils;
class UpdateParams;
class AMODController;
class OD_Trip;


/**
 * Basic Person class.
 *
 * \author Seth N. Hetu
 * \author Wang Xinyuan
 * \author Luo Linbo
 * \author Li Zhemin
 * \author Xu Yan
 * \author Harish Loganathan
 * \author zhang huai peng
 * \author Yao Jin
 * \author Vahid Saber
 *
 * A person may perform one of several roles which
 *  change over time. For example: Drivers, Pedestrians, and Passengers are
 *  all roles which a Person may fulfill.
 */
class Person : public sim_mob::Agent {
public:
	bool tripchainInitialized;
	///The "src" variable is used to help flag how this person was created.
	explicit Person(const std::string& src, const MutexStrategy& mtxStrat, int id=-1, std::string databaseID = "");
	explicit Person(const std::string& src, const MutexStrategy& mtxStrat, const std::vector<sim_mob::TripChainItem*>& tc);
	virtual ~Person();
	void initTripChain();

	void setConfigProperties(const std::map<std::string, std::string>& props)
	{
		this->configProperties = props;
	}

	const std::map<std::string, std::string>& getConfigProperties()
	{
		return this->configProperties;
	}

	void setNextPathPlanned(bool value)
	{
		nextPathPlanned = value;
	}

	bool getNextPathPlanned()
	{
		return nextPathPlanned;
	}

	long getLastUpdatedFrame() const
	{
		return lastUpdatedFrame;
	}

	void setLastUpdatedFrame(long lastUpdatedFrame)
	{
		this->lastUpdatedFrame = lastUpdatedFrame;
	}

	/**Clears the map configProperties which contains the configuration properties*/
	void clearConfigProperties()
	{
		this->configProperties.clear();
	}

	/**The agent's start node*/
	WayPoint originNode;

	/**The agent's end node*/
	WayPoint destNode;

	/**Indicates if the agent is queuing*/
	bool isQueuing;

	/**The distance to the end of the segment*/
	double distanceToEndOfSegment;

	/**The time taken to drive to the end of the link*/
	double drivingTimeToEndOfLink;

	/**Holds the road segment travel time*/
	RdSegTravelStat currRdSegTravelStats;

	/**Holds the link travel time*/
	LinkTravelStats currLinkTravelStats;

	/**Stores the link travel times with link exit time as the key*/
	std::map<double, LinkTravelStats> linkTravelStatsMap;

	/**
	 * Inserts the LinkTravelStats into the map
	 * @param ts the LinkTravelStats to be added
	 * @param exitTime the time of exiting the link
	 */
	void addToLinkTravelStatsMap(LinkTravelStats ts, double exitTime);

	/**
	 * Clears the flag indicating that the agent is marked for removal
	 */
	void clearToBeRemoved()
	 {
	 	toRemoved = false;
	 }

	//Person objects are spatial in nature
	virtual bool isNonspatial() { return false; }

	void handleAMODEvent(sim_mob::event::EventId id,
            sim_mob::event::Context ctxId,
            sim_mob::event::EventPublisher* sender,
            const AMOD::AMODEventArgs& args);

	/**
	 * Ask this person to re-route to the destination with the given set of blacklisted RoadSegments
	 * If the Agent cannot complete this new route, it will fall back onto the old route.
	 *
	 * @param blacklisted the black-listed road segments
	 */
	virtual void rerouteWithBlacklist(const std::vector<const sim_mob::RoadSegment*>& blacklisted);

	///Load a Person's config-specified properties, creating a placeholder trip chain if
	/// requested.
	virtual void load(const std::map<std::string, std::string>& configProps);

	/**
	 * Update a Person's subscription list.
	 * @return
	 */
	virtual std::vector<BufferedBase *> buildSubscriptionList();

    //interfaces dynamically to modify the trip chain
    bool insertATripChainItem(TripChainItem* before, TripChainItem* newone);
    bool deleteATripChainItem(TripChainItem* del);
    bool replaceATripChainItem(TripChainItem* rep, TripChainItem* newone);

    bool insertTripBeforeCurrentTrip(Trip* newone);
    bool insertSubTripBeforeCurrentSubTrip(SubTrip* newone);

    //modify trip chain so that a new item is inserted between walk and bus travel mode
    void simplyModifyTripChain(std::vector<TripChainItem*>& tripChain);

    ///Change the role of this person: Driver, Passenger, Pedestrian
    void changeRole(sim_mob::Role* newRole);
    sim_mob::Role* getRole() const;
    sim_mob::Role* getPrevRole() const;
    // set NextRole
    void setNextRole(sim_mob::Role* newRole);
    // get NextRole
    sim_mob::Role* getNextRole() const;
    bool updatePersonRole(sim_mob::Role* newRole = 0);
    // find Person's NextRole
    bool findPersonNextRole();

    virtual void setStartTime(unsigned int value);

    /**
     * insert a waiting activity before bus travel
     * @param tripChain is the reference to current trip chain
     */
    void insertWaitingActivityToTrip();
    /**
     * alters trip chain in accordance to route choice for PT trips
     */
    void convertODsToTrips();
    /**
     * creates subtrips for each leg of PT route choice made by person
     */
    bool makeODsToTrips(SubTrip* curSubTrip, std::vector<sim_mob::SubTrip>& newSubTrips, const std::vector<sim_mob::OD_Trip>& matchedTrips);
    /**
     * assigns ids to subtrips
     */
    void assignSubtripIds();

    // update nextTripChainItem, used only for NextRole
	bool updateNextTripChainItem();
	// update nextSubTrip, used only for NextRole
	bool updateNextSubTrip();
    ///Check if any role changing is required.
    /// "nextValidTimeMS" is the next valid time tick, which may be the same at this time tick.
    Entity::UpdateStatus checkTripChain();
	//update origin and destination node based on the trip, subtrip or activity given
	bool updateOD(sim_mob::TripChainItem* tc, const sim_mob::SubTrip* subtrip = 0);

	///get this person's trip chain
	const std::vector<TripChainItem*>& getTripChain() const {
		return tripChain;
	}

	///Set this person's trip chain
	void setTripChain(const std::vector<TripChainItem *>& tripChain);

	/*	const sim_mob::Link* getCurrLink() const;
	 void setCurrLink(sim_mob::Link* link);*/
	int laneID;
	int initSegId;
	int initDis;
	int initSpeed;

	const std::string& getAgentSrc() const {
		return agentSrc;
	}

	SubTrip* getNextSubTripInTrip();
	TripChainItem* findNextItemInTripChain();

	const std::string& getDatabaseId() const {
		return databaseID;
	}

	void setDatabaseId(const std::string& databaseId) {
		databaseID = databaseId;
	}

	// set Person's characteristics by some distribution
	void setPersonCharacteristics();

	bool isResetParamsRequired() const {
		return resetParamsRequired;
	}

	void setResetParamsRequired(bool resetParamsRequired) {
		this->resetParamsRequired = resetParamsRequired;
	}
	// get boarding time secs for this person
	double getBoardingCharacteristics() const { return boardingTimeSecs; }
	// get alighting time secs for this person
	double getAlightingCharacteristics() const { return alightingTimeSecs; }

	// pointer to current item in trip chain
    std::vector<TripChainItem*>::iterator currTripChainItem;
    //pointer to current subtrip in the current trip (if  current item is trip)
    std::vector<SubTrip>::iterator currSubTrip;

    // pointer to next item in trip chain
    std::vector<TripChainItem*>::iterator nextTripChainItem;
    //pointer to next subtrip in the current trip (if  current item is trip)
    std::vector<SubTrip>::const_iterator nextSubTrip;

	double getRemainingTimeThisTick() const {
		return remainingTimeThisTick;
	}

	void setRemainingTimeThisTick(double remainingTimeThisTick) {
		this->remainingTimeThisTick = remainingTimeThisTick;
	}

    const sim_mob::SegmentStats* requestedNextSegStats;  //Used by confluxes and movement facet of roles to move this person in the medium term

    enum Permission //to be renamed later
   	{
   		NONE=0,
   		GRANTED,
   		DENIED
   	};
    Permission canMoveToNextSegment;

    //Used for passing various debug data. Do not rely on this for anything long-term.
    //std::string specialStr;

    std::stringstream debugMsgs;
    int client_id;

	// amod
	std::string amodId;
	void setPath(std::vector<WayPoint>& path);
	std::vector<WayPoint> amodPath;
	std::string amodPickUpSegmentStr;
	double amodSegmLength;
	double amodSegmLength2;
	std::string amodDropOffSegmentStr;
	std::string amdoTripId;
	std::string parkingNode;
    AMOD::AMODEventPublisher eventPub;

    void handleAMODArrival();
    void handleAMODPickup();

    enum Status {
    	IN_CAR_PARK = 0,
    	ON_THE_ROAD,
    	REPLACED
    };

    Status currStatus;

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

	void advanceToNextRole();
	///	container to store all the metrics of the trips (subtrip actually)
	 /*
	  * The implementation inserts information at subtrip resolution while preday will require Trip-level metrics.
	  *  So whenever all subtrips of a trip are done (subTripTravelMetrics) and it is time to change the tripchainitem
	  *  (in Pesron class) an aggregate function will create a new entry in tripTravelMetrics from subTripTravelMetrics
	  *  items. subTripTravelMetrics items are cleared then.
	  */
	 std::vector<TravelMetric> tripTravelMetrics;

	 /**
	  * A version of serializer for subtrip level travel time.
	  * @param subtripMetrics input metrics
	  * @param currTripChainItem current TripChainItem
	  * @param currSubTrip current SubTrip for which subtripMetrics is collected
	  */
	 void serializeSubTripChainItemTravelTimeMetrics(
			 const TravelMetric& subtripMetrics,
			 std::vector<TripChainItem*>::iterator currTripChainItem,
			 std::vector<SubTrip>::iterator currSubTrip) const;


	 /**
	  * subtrip level travel metrics
	  * NOTE: Currently Unused
	  */
	 std::vector<TravelMetric> subTripTravelMetrics;

	/**
	 * get the measurements stored in subTripTravelMetrics and add them up into a new entry in tripTravelMetrics.
	 * call this method whenever a subtrip is done.
	  * NOTE: Currently Unused
	 */
	void aggregateSubTripMetrics();

	/**
	 * add the given TravelMetric to subTripTravelMetrics container
	  * NOTE: Currently Unused
	 */
	void addSubtripTravelMetrics(TravelMetric & value);

	/**
	 * Serializer for Trip level travel time
	  * NOTE: Currently Unused
	 */
	 void serializeTripTravelTimeMetrics();

	 /**
	  * This is called by movement facet's destructor of non-activity role
	  * NOTE: Currently Unused
	  */
	 void serializeCBD_SubTrip(const TravelMetric &metric);

	 /**
	 * This is called by  movement facet's destructor activity role
	 * NOTE: Currently Unused
	 */
	void serializeCBD_Activity(const TravelMetric &metric);

	boost::mt19937& getGenerator()
	{
		return currWorkerProvider->getGenerator();
	}

private:

	 /**
	  * prints the trip chain item types of each item in tripChain
	  */
	 void printTripChainItemTypes() const;

	/**Stores the configuration properties of the agent loaded from the XML configuration file*/
	std::map<std::string, std::string> configProperties;

	/**Indicates if the detailed path for the current sub-trip is already planned*/
	bool nextPathPlanned;

	/**Stores the frame number in which the previous update of this agent took place*/
	long lastUpdatedFrame;

	///Have we registered to receive commsim-related messages?
	bool commEventRegistered;
protected:
	virtual bool frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);


	//Inherited from EventListener.
	virtual void onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);


	//Inherited from MessageHandler.
	 virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);


	virtual void rerouteWithBlacklist(const std::vector<const sim_mob::RoadSegment *>& blacklisted);

private:
	//to indicate that Role's updateParams has to be reset.
	bool resetParamsRequired;

	bool advanceCurrentTripChainItem();
	bool advanceCurrentSubTrip();
	std::vector<sim_mob::SubTrip>::iterator resetCurrSubTrip();

    //Properties
    sim_mob::Role* prevRole; ///< To be deleted on the next time tick.
    sim_mob::Role* currRole;
    sim_mob::Role* nextRole; //do not be misled. this variable is only temporary and will not be used to update the currRole

    //Can be helpful for debugging
    std::string agentSrc;

    int currTripChainSequenceNumber;
    std::vector<TripChainItem*> tripChain;

    //to mark the first call to update function
    bool firstTick;

    //Used by confluxes to move the person for his tick duration across link and sub-trip boundaries
    double remainingTimeThisTick;

    std::string databaseID;
    // person's age
    unsigned int age;
    // person's boarding time secs
    double boardingTimeSecs;
    // person's alighting time secs
    double alightingTimeSecs;

    // current lane and segment are needed for confluxes to track this person
	const sim_mob::Lane* currLane;
	const sim_mob::SegmentStats* currSegStats;
	const sim_mob::Link* nextLinkRequired;

    friend class PartitionManager;
    friend class BoundaryProcessor;

public:
	virtual void pack(PackageUtils& packageUtil) CHECK_MPI_THROW;
	virtual void unpack(UnPackageUtils& unpackageUtil) CHECK_MPI_THROW;

	virtual void packProxy(PackageUtils& packageUtil) CHECK_MPI_THROW;
	virtual void unpackProxy(UnPackageUtils& unpackageUtil) CHECK_MPI_THROW;

	friend class Conflux;

	std::string busLine; //tmp addition for debugging ~ Harish
	int bustripnum; //tmp addition for debugging ~ Harish
};

}
