//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <string>
#include <vector>

#include "conf/settings/DisableMPI.h"
#include "entities/Agent.hpp"
#include "entities/conflux/Conflux.hpp"
#include "entities/conflux/SegmentStats.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "util/LangHelpers.hpp"
#include "util/Profiler.hpp"
#include <boost/foreach.hpp>
namespace sim_mob
{

class Role;
class TripChainItem;
class SubTrip;
class PartitionManager;
class PackageUtils;
class UnPackageUtils;
class UpdateParams;

/// simple structure used to collect travel time information
struct TravelMetric
{
	WayPoint origin,destination;
	DailyTime startTime,endTime;
	uint32_t travelTime;
	bool started,finalized,valid;
	TravelMetric():started(false),finalized(false),valid(false){}
};

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


	//Person objects are spatial in nature
	virtual bool isNonspatial() { return false; }

	///Reroute to the destination with the given set of blacklisted RoadSegments.
	///If the Agent cannot complete this new route, it will fall back onto the old route.
	virtual void rerouteWithBlacklist(const std::vector<const sim_mob::RoadSegment*>& blacklisted);

	///Load a Person's config-specified properties, creating a placeholder trip chain if
	/// requested.
	virtual void load(const std::map<std::string, std::string>& configProps);

    ///Update a Person's subscription list.
    virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

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
    // set NextRole
    void setNextRole(sim_mob::Role* newRole);
    // get NextRole
    sim_mob::Role* getNextRole() const;
    bool updatePersonRole(sim_mob::Role* newRole = 0);
    // find Person's NextRole
    bool findPersonNextRole();

    /**
     * insert a waiting activity before bus travel
     * @param tripChain is the reference to current trip chain
     */
    void insertWaitingActivityToTrip(std::vector<TripChainItem*>& tripChain);

    // update nextTripChainItem, used only for NextRole
	bool updateNextTripChainItem();
	// update nextSubTrip, used only for NextRole
	bool updateNextSubTrip();
    ///Check if any role changing is required.
    /// "nextValidTimeMS" is the next valid time tick, which may be the same at this time tick.
    Entity::UpdateStatus checkTripChain();
    bool changeRoleRequired(sim_mob::Role & currRole,sim_mob::SubTrip &currSubTrip)const;//todo depricate later
    bool changeRoleRequired_Trip /*sim_mob::Trip &trip*/
	() const;
	bool changeRoleRequired_Activity /*sim_mob::Activity &activity*/
	() const;
	bool changeRoleRequired(sim_mob::TripChainItem& tripChinItem) const;
	//update origin and destination node based on the trip, subtrip or activity given
	bool updateOD(sim_mob::TripChainItem* tc, const sim_mob::SubTrip* subtrip =
			0);

	///get this person's trip chain
	const std::vector<TripChainItem*>& getTripChain() const {
		return tripChain;
	}

	///Set this person's trip chain
	void setTripChain(const std::vector<TripChainItem*>& tripChain) {
		this->tripChain = tripChain;
	}

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

	const std::vector<WayPoint>& getCurrPath() const {
		return currPath;
	}

	void setCurrPath(const std::vector<WayPoint>& currPath) {
		this->currPath = currPath;
	}

	void clearCurrPath() {
		this->currPath.clear();
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
	  *  so whenever all subtrips of a trip are done (subTripTravelMetrics) and it is time to change the tripchainitem(in Pesron class)
	  *  an aggregate function will create a new entry in tripTravelMetrics from subTripTravelMetrics items.
	  *  subTripTravelMetrics items are cleared then.
	  */
	 std::vector<TravelMetric> tripTravelMetrics;
	 std::vector<TravelMetric> subTripTravelMetrics;
	///	get the measurements stored in subTripTravelMetrics and add them up into a new entry in tripTravelMetrics
	///	call this method whenever a subtrip is done.
	void aggregateSubTripMetrics()
	{
		TravelMetric newTripMetric;
//		if(subTripTravelMetrics.begin() == subTripTravelMetrics.end())
//		{
//			throw std::runtime_error("subTrip level TravelMetrics is missing");
//		}
		TravelMetric item(*subTripTravelMetrics.begin());
		newTripMetric.startTime = item.startTime;//first item
		newTripMetric.origin = item.origin;
		 BOOST_FOREACH(item, subTripTravelMetrics)
		 {
			 newTripMetric.travelTime += item.travelTime;
		 }
		 newTripMetric.endTime = item.endTime;
		 newTripMetric.destination = item.destination;
		 subTripTravelMetrics.clear();
		 tripTravelMetrics.push_back(newTripMetric);
	}
//	 std::vector<TravelMetric> & getTipTravelMetrics()
//	 {
//		 return tripTravelMetrics;
//	 }
//
//	 void addTripTravelMetrics(TravelMetric & value){
//		 tripTravelMetrics.push_back(value);
//	 }
//
//	 std::vector<TravelMetric> & getSubTipTravelMetrics()
//	 {
//		 return subTripTravelMetrics;
//	 }
//
	 void addSubtripTravelMetrics(TravelMetric & value){
		 subTripTravelMetrics.push_back(value);
	 }

	 void serializeTripTravelTimeMetrics()
	 {
		 sim_mob::BasicLogger & csv = sim_mob::Logger::log("person_travel_time");
		 BOOST_FOREACH(TravelMetric item, tripTravelMetrics)
		 {
			 csv << this->getId() << "," <<
					 item.origin.node_->getID() << ","
					 << item.destination.node_->getID() << ","
					 << item.startTime.getRepr_() << ","
					 << item.endTime.getRepr_() << ","
					 << (item.endTime - item.startTime).getRepr_()
					 << "\n";
		 }
		 tripTravelMetrics.clear();
	 }



protected:
	virtual bool frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);


	//Inherited from EventListener.
	virtual void onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);


	//Inherited from MessageHandler.
	 virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);


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
    bool first_update_tick;

    //Used by confluxes to move the person for his tick duration across link and sub-trip boundaries
    double remainingTimeThisTick;

    friend class PartitionManager;
    friend class BoundaryProcessor;

    std::string databaseID;
    // person's age
    unsigned int age;
    // person's boarding time secs
    double boardingTimeSecs;
    // person's alighting time secs
    double alightingTimeSecs;
    std::vector<WayPoint> currPath;

    // current lane and segment are needed for confluxes to track this person
	const sim_mob::Lane* currLane;
	const sim_mob::SegmentStats* currSegStats;

	const sim_mob::Link* nextLinkRequired;

public:
	virtual void pack(PackageUtils& packageUtil) CHECK_MPI_THROW;
	virtual void unpack(UnPackageUtils& unpackageUtil) CHECK_MPI_THROW;

	virtual void packProxy(PackageUtils& packageUtil) CHECK_MPI_THROW;
	virtual void unpackProxy(UnPackageUtils& unpackageUtil) CHECK_MPI_THROW;

	friend class Conflux;
};

}
