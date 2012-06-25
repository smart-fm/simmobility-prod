/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "GenConfig.h"

#include "Agent.hpp"
#include "roles/Role.hpp"
#include "roles/driver/Driver.hpp"
#include "buffering/Shared.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/misc/TripChain.hpp"

namespace sim_mob
{

class TripChainItem;

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
 *
 * A person may perform one of several roles which
 *  change over time. For example: Drivers, Pedestrians, and Passengers are
 *  all roles which a Person may fulfill.
 */
class Person : public sim_mob::Agent {
public:
	explicit Person(const MutexStrategy& mtxStrat, int id=-1);
	virtual ~Person();

	///Generate a person from a PendingEntity. Currently only works for Drivers/Pedestrians
	static Person* GeneratePersonFromPending(const PendingEntity& p);

	///Update Person behavior
	virtual Entity::UpdateStatus update(frame_t frameNumber);
    ///Update a Person's subscription list.
    virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);
    ///Change the role of this person: Driver, Passenger, Pedestrian
    void changeRole(sim_mob::Role* newRole);
    sim_mob::Role* getRole() const;
    ///Check if any role changing is required.
    Entity::UpdateStatus checkAndReactToTripChain(unsigned int currTimeMS);

    ///get this person's trip chain
    std::vector<TripChainItem*> getTripChain() const
    {
        return tripChain;
    }

    ///Set this person's trip chain
    void setTripChain(const std::vector<TripChainItem*> tripChain)
    {
        this->tripChain = tripChain;
    }

    void getFirstTripInChain(std::vector<sim_mob::SubTrip*>::iterator subTripPtr);

    std::vector<TripChainItem*>::iterator currTripChainItem; // pointer to current item in trip chain
    std::vector<SubTrip*>::iterator currSubTrip; //pointer to current subtrip in the current trip (if  current item is trip)

    //Used for passing various debug data. Do not rely on this for anything long-term.
    std::string specialStr;

private:
    //Properties
    sim_mob::Role* prevRole; ///< To be deleted on the next time tick.
    sim_mob::Role* currRole;
    //sim_mob::TripChainItem* currTripChainItem;
    int currTripChainSequenceNumber;
    std::vector<TripChainItem*> tripChain;
    bool firstFrameTick;
    ///Determines if frame_init() has been done.
    friend class PartitionManager;
    friend class BoundaryProcessor;
    
    //add by xuyan
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil);
	virtual void unpack(UnPackageUtils& unpackageUtil);

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);

#endif
};





}
