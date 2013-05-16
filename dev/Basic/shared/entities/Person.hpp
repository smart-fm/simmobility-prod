/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <map>
#include <string>
#include <vector>

#include "conf/settings/DisableMPI.h"

#include "entities/Agent.hpp"
#include "roles/Role.hpp"
#include "buffering/Shared.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/conflux/Conflux.hpp"

namespace sim_mob
{

class TripChainItem;
class SubTrip;

#ifndef SIMMOB_DISABLE_MPI
class PartitionManager;
class PackageUtils;
class UnPackageUtils;
#endif

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

	//Update Person behavior (old)
	//virtual Entity::UpdateStatus update(timeslice now);

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
    bool updatePersonRole();
    ///Check if any role changing is required.
    /// "nextValidTimeMS" is the next valid time tick, which may be the same at this time tick.
    Entity::UpdateStatus checkTripChain(uint32_t currTimeMS);
    bool changeRoleRequired(sim_mob::Role & currRole,sim_mob::SubTrip &currSubTrip)const;//todo depricate later
    bool changeRoleRequired_Trip(/*sim_mob::Trip &trip*/) const;
    bool changeRoleRequired_Activity(/*sim_mob::Activity &activity*/) const;
    bool changeRoleRequired(sim_mob::TripChainItem &tripChinItem) const;
    //update origin and destination node based on the trip, subtrip or activity given
    bool updateOD(sim_mob::TripChainItem *tc ,const sim_mob::SubTrip *subtrip = 0);
    ///get this person's trip chain
    std::vector<TripChainItem*>& getTripChain()
    {
        return tripChain;
    }

    ///Set this person's trip chain
    void setTripChain(const std::vector<TripChainItem*>& tripChain)
    {
        this->tripChain = tripChain;
    }

/*	const sim_mob::Link* getCurrLink() const;
	void setCurrLink(sim_mob::Link* link);*/
	
	int laneID;
	const std::string& getAgentSrc() const { return agentSrc; }

    SubTrip* getNextSubTripInTrip();
    TripChainItem* findNextItemInTripChain();

	const std::string& getDatabaseId() const {
		return databaseID;
	}

	void setDatabaseId(const std::string& databaseId) {
		databaseID = databaseId;
	}

	double getRemainingTimeThisTick() const {
		return remainingTimeThisTick;
	}

	void setRemainingTimeThisTick(double remainingTimeThisTick) {
		this->remainingTimeThisTick = remainingTimeThisTick;
	}

    std::vector<TripChainItem*>::iterator currTripChainItem; // pointer to current item in trip chain

    std::vector<SubTrip>::iterator currSubTrip; //pointer to current subtrip in the current trip (if  current item is trip)

    const sim_mob::RoadSegment* requestedNextSegment;  //Used by confluxes and movement facet of roles to move this person in the medium term

    enum Permission //to be renamed later
    	{
    		NONE=0,
    		GRANTED,
    		DENIED
    	};
    Permission canMoveToNextSegment;

    //Used for passing various debug data. Do not rely on this for anything long-term.
    std::string specialStr;

    std::stringstream debugMsgs;

protected:
	virtual bool frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);

private:
	//Very risky:
	UpdateParams* curr_params;


	bool advanceCurrentTripChainItem();
	bool advanceCurrentSubTrip();
	std::vector<sim_mob::SubTrip>::iterator resetCurrSubTrip();

    //Properties
    sim_mob::Role* prevRole; ///< To be deleted on the next time tick.
    sim_mob::Role* currRole;

    //Can be helpful for debugging
    std::string agentSrc;

    int currTripChainSequenceNumber;
    std::vector<TripChainItem*> tripChain;

    //to mark the first call to update function
    bool first_update_tick;

    //Used by confluxes to move the person for his tick duration across link and sub-trip boundaries
    double remainingTimeThisTick;

    ///Determines if frame_init() has been done.
    friend class PartitionManager;
    friend class BoundaryProcessor;

    std::string databaseID;

#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil);
	virtual void unpack(UnPackageUtils& unpackageUtil);

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);

#endif

	friend class Conflux;
};

}
