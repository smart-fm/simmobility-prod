/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <map>
#include <string>
#include <vector>

#include "GenConfig.h"

#include "entities/Agent.hpp"
#include "roles/Role.hpp"
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
 *  all roles which a Person may fulfil.
 */
class Person : public sim_mob::Agent {
public:
	///The "src" variable is used to help flag how this person was created.
	explicit Person(const std::string& src, const MutexStrategy& mtxStrat,unsigned int id=-1);
	explicit Person(const std::string& src, const MutexStrategy& mtxStrat, std::vector<sim_mob::TripChainItem*> tc);
	virtual ~Person();

	///Update Person behavior
	virtual Entity::UpdateStatus update(frame_t frameNumber);

	///Load a Person's config-specified properties, creating a placeholder trip chain if
	/// requested.
	virtual void load(const std::map<std::string, std::string>& configProps);

    ///Update a Person's subscription list.
    virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

    ///Change the role of this person: Driver, Passenger, Pedestrian
    void changeRole(sim_mob::Role* newRole);
    sim_mob::Role* getRole() const;

    ///Check if any role changing is required.
    /// "nextValidTimeMS" is the next valid time tick, which may be the same at this time tick.
    Entity::UpdateStatus checkAndReactToTripChain(unsigned int currTimeMS, unsigned int nextValidTimeMS);

    ///get this person's trip chain
    std::vector<TripChainItem*>& getTripChain()
    {
        return tripChain;
    }

    ///Set this person's trip chain
    void setTripChain(std::vector<TripChainItem*>& tripChain)
    {
        this->tripChain = tripChain;
    }

/*	const sim_mob::Link* getCurrLink() const;
	void setCurrLink(sim_mob::Link* link);*/

    void getNextSubTripInTrip();
    void findNextItemInTripChain();

    TripChainItem* currTripChainItem; // pointer to current item in trip chain
    SubTrip* currSubTrip; //pointer to current subtrip in the current trip (if  current item is trip)

    //Used for passing various debug data. Do not rely on this for anything long-term.
    std::string specialStr;

private:
    //Internal update functionality
    void update_time(frame_t frameNumber, unsigned int currTimeMS, Entity::UpdateStatus& retVal);


    //Properties
    sim_mob::Role* prevRole; ///< To be deleted on the next time tick.
    sim_mob::Role* currRole;

    //Can be helpful for debugging
    std::string agentSrc;


    int currTripChainSequenceNumber;
    std::vector<TripChainItem*> tripChain;
    bool firstFrameTick;
    ///Determines if frame_init() has been done.
    friend class PartitionManager;
    friend class BoundaryProcessor;

#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil);
	virtual void unpack(UnPackageUtils& unpackageUtil);

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);

#endif
};

}
