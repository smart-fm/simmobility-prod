//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <string>
#include <vector>

#include "conf/settings/DisableMPI.h"
#include "entities/conflux/Conflux.hpp"
#include "entities/Agent.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "util/LangHelpers.hpp"

namespace sim_mob
{

class Role;
class TripChainItem;
class SubTrip;
class PartitionManager;
class PackageUtils;
class UnPackageUtils;
class UpdateParams;



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
	explicit Person(const std::string& src, const MutexStrategy& mtxStrat, std::vector<sim_mob::TripChainItem*> tc);
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

    //combine together all the pedestrian roles and insert waiting activity at bus stop
    void adjustTripChainsForMedium(std::vector<TripChainItem*>& tripChain);

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

    // update nextTripChainItem, used only for NextRole
	bool updateNextTripChainItem();
	// update nextSubTrip, used only for NextRole
	bool updateNextSubTrip();
    ///Check if any role changing is required.
    /// "nextValidTimeMS" is the next valid time tick, which may be the same at this time tick.
    Entity::UpdateStatus checkTripChain(uint32_t currTimeMS);
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
	std::vector<TripChainItem*>& getTripChain() {
		return tripChain;
	}

	///Set this person's trip chain
	void setTripChain(const std::vector<TripChainItem*>& tripChain) {
		this->tripChain = tripChain;
	}

	/*	const sim_mob::Link* getCurrLink() const;
	 void setCurrLink(sim_mob::Link* link);*/
	int laneID;

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
	double getBoardingCharacteristics() const { return BOARDING_TIME_SEC; }
	// get alighting time secs for this person
	double getAlightingCharacteristics() const { return ALIGTHING_TIME_SEC; }

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

    const sim_mob::RoadSegment* requestedNextSegment;  //Used by confluxes and movement facet of roles to move this person in the medium term

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

protected:
	virtual bool frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);

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
    double BOARDING_TIME_SEC;
    // person's alighting time secs
    double ALIGTHING_TIME_SEC;
    std::vector<WayPoint> currPath;

public:
	virtual void pack(PackageUtils& packageUtil) CHECK_MPI_THROW;
	virtual void unpack(UnPackageUtils& unpackageUtil) CHECK_MPI_THROW;

	virtual void packProxy(PackageUtils& packageUtil) CHECK_MPI_THROW;
	virtual void unpackProxy(UnPackageUtils& unpackageUtil) CHECK_MPI_THROW;

	friend class Conflux;
};

}
