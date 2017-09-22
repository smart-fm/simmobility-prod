//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "conf/settings/DisableMPI.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/random.hpp>

#include "util/LangHelpers.hpp"
#include "entities/vehicle/VehicleBase.hpp"
#include "entities/UpdateParams.hpp"
#include "workers/Worker.hpp"
#include "logging/Log.hpp"
#include "DriverRequestParams.hpp"
#include "RoleFacets.hpp"

namespace sim_mob
{

#ifndef SIMMOB_DISABLE_MPI
class PartitionManager;
class PackageUtils;
class UnPackageUtils;
#endif

template<class PARAM>
class UpdateWrapper
{
protected:
	PARAM dataParam;

public:

	UpdateWrapper()
	{
	}

	PARAM &getParams()
	{
		return dataParam;
	}

	void setParams(PARAM &value)
	{
		dataParam = value;
	}
};

/**
 * Role that a person may fulfill.
 *
 * \author Seth N. Hetu
 * \author Xu Yan
 * \author Vahid
 *
 *
 * Allows Person agents to swap out roles easily,
 * without re-creating themselves or maintaining temporarily irrelevant data.
 *
 * \note
 * For now, this class is very simplistic.
 */
template<class PERSON>
class Role
{
protected:
	/**The person who is playing the role*/
	PERSON *parent;

	/**The resource used by the person to play out the role*/
	VehicleBase* currResource;

	/**The behaviour for the role*/
	BehaviorFacet* behaviorFacet;

	/**The movement for the role*/
	MovementFacet* movementFacet;

	/* TODO: totalTravelTimeMS and arrivalTimeMS does not belong here.
	 * This has to be re-factored and moved into relevant sub classes of role after July workshop 2015. ~Harish*/
	unsigned int totalTravelTimeMS;
	unsigned int arrivalTimeMS;

	/**The mode of travel*/
	const std::string mode;

	/**Seed for random number generator*/
	int dynamicSeed;

	NullableOutputStream Log()
	{
		return NullableOutputStream(parent->currWorkerProvider->getLogFile());
	}

public:
	/**Defines the various types of roles*/
	enum Type
	{
		RL_UNKNOWN = 0,
		RL_DRIVER,
		RL_BIKER,
		RL_PEDESTRIAN,
		RL_TRAVELPEDESTRIAN,
		RL_BUSDRIVER,
		RL_ACTIVITY,
		RL_PASSENGER,
		RL_WAITBUSACTIVITY,
		RL_WAITTAXIACTIVITY,
		RL_TRAINPASSENGER,
		RL_CARPASSENGER,
		RL_PRIVATEBUSPASSENGER,
		RL_TRAINDRIVER,
		RL_WAITTRAINACTIVITY,
		RL_TAXIPASSENGER,
		RL_TRUCKER_LGV,
		RL_TRUCKER_HGV
	};

	/**Defines the various types of requests*/
	enum request
	{
		REQUEST_NONE = 0,
		REQUEST_DECISION_TIME,
		REQUEST_STORE_ARRIVING_TIME
	};

	/**The role name*/
	const std::string name;

	/**The type of the role*/
	const Type roleType;

	explicit Role(PERSON *person, std::string roleName = std::string(), Role<PERSON>::Type roleType_ = RL_UNKNOWN) :
	parent(person), currResource(nullptr), name(roleName), mode(mode), roleType(roleType_), behaviorFacet(nullptr),
	movementFacet(nullptr), dynamicSeed(0), totalTravelTimeMS(0), arrivalTimeMS(0)
	{
	}

	explicit Role(PERSON *person, sim_mob::BehaviorFacet* behavior = nullptr, sim_mob::MovementFacet* movement = nullptr,
				std::string roleName = std::string(), Role<PERSON>::Type roleType_ = RL_UNKNOWN) :
	parent(person), currResource(nullptr), name(roleName), roleType(roleType_), behaviorFacet(behavior), movementFacet(movement),
	dynamicSeed(0), totalTravelTimeMS(0), arrivalTimeMS(0)
	{
	}

	virtual ~Role()
	{
		safe_delete_item(behaviorFacet);
		safe_delete_item(movementFacet);
		safe_delete_item(currResource);
	}

	/**
	 * This method enables the creation of roles. This is done by copying the role prototypes.
	 *
     * @param parent the person who will play the role that is to be created
	 *
     * @return the created role
     */
	virtual Role<PERSON>* clone(PERSON* parent) const
	{
		return nullptr;
	}

	/**
	 * Builds the list of subscriptions that need to be managed
	 * 
     * @return list of parameters that expect their subscriptions to be managed
     */
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams() = 0;

	/**
	 * Create the UpdateParams which will hold all the temporary information for this time tick.
	 *
     * @param now the time frame for which the update parameters are to be created
     */
	virtual void make_frame_tick_params(timeslice now) = 0;

	const std::string getMode()
	{
		switch (roleType)
		{
		case RL_UNKNOWN: return "NA";
		case RL_DRIVER: return "Car";
		case RL_BIKER: return "Motorcycle";
		case RL_PEDESTRIAN: return "Walk";
		case RL_BUSDRIVER: return "Bus";
		case RL_ACTIVITY: return "Activity";
		case RL_PASSENGER: return "BusTravel";
		case RL_WAITBUSACTIVITY: return "WaitingBusActivity";
		case RL_TRUCKER_HGV: return "HGV";
		case RL_TRUCKER_LGV: return "LGV";
		}
	}

	std::string getRoleName()const
	{
		return name;
	}

	/**
	 * Provides information to the MovementFacet object passed as argument. Such information can be provided by passing the
	 * 'this' pointer as an argument to one of the MovementFacet object's methods.
	 * Note:This twisting was originally invented to avoid dynamic_cast(s)
	 *
     * @param mFacet
     */
	virtual void handleUpdateRequest(MovementFacet* mFacet)
	{
	}

	/**
	 * This method asks the role to re-route its current sub-trip, avoiding the given blacklisted links.
	 * This should keep the Role at its current position, but change all Links after this one.
	 * NOTE: If no alternative route exists, this Role's current route will remain unchanged.
	 * This function is somewhat experimental; use it with caution. Currently only implemented by the Driver class.
     * @param blacklisted the list of black listed links
     */
	virtual void rerouteWithBlacklist(const std::vector<const Link*>& blacklisted)
	{
	}

	/**Collect current travel time*/
	virtual void collectTravelTime()
	{
	}

	/**
	 * Event handler which provides a chance to handle event transfered from parent agent.
	 * @param sender pointer for the event producer.
	 * @param id event identifier.
	 * @param args event arguments.
	 */
	virtual void onParentEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args)
	{
	}

	/**
	 * Message handler which provides a chance to handle message transfered from parent agent.
	 * @param type of the message.
	 * @param message data received.
	 */
	virtual void HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message)
	{
	}

	virtual std::vector<sim_mob::BufferedBase*> getDriverInternalParams()
	{
		return std::vector<BufferedBase*>();
	}

	/**
	 * This method returns a request list for asynchronous communication.
	 * Subclasses of Role should override this method if they want to enable asynchronous communication.
	 * NOTE: This function is only used by the Driver class, but it's required here
	 * due to the way we split Driver into the short-term folder
     * @return a request list for asynchronous communication
     */
	virtual sim_mob::DriverRequestParams getDriverRequestParams()
	{
		return sim_mob::DriverRequestParams();
	}

	VehicleBase* getResource() const
	{
		return currResource;
	}

	void setResource(VehicleBase* currResource)
	{
		this->currResource = currResource;
	}

	PERSON* getParent() const
	{
		return parent;
	}

	void setParent(PERSON *person)
	{
		parent = person;
	}

	BehaviorFacet* Behavior() const
	{
		return behaviorFacet;
	}

	MovementFacet* Movement() const
	{
		return movementFacet;
	}

	void setTravelTime(unsigned int time)
	{
		totalTravelTimeMS = time;
	}

	const unsigned int getTravelTime() const
	{
		return totalTravelTimeMS;
	}

	void setArrivalTime(unsigned int time)
	{
		arrivalTimeMS = time;
	}

	const unsigned int getArrivalTime() const
	{
		return arrivalTimeMS;
	}


	/* TODO: totalTravelTimeMS and arrivalTimeMS does not belong here.
	 * This has to be re-factored and moved into relevant sub classes of role after July workshop 2015. ~Harish*/
#ifndef SIMMOB_DISABLE_MPI
	friend class sim_mob::PartitionManager;

	virtual void pack(PackageUtils& packageUtil) = 0;
	virtual void unpack(UnPackageUtils& unpackageUtil) = 0;

	virtual void packProxy(PackageUtils& packageUtil) = 0;
	virtual void unpackProxy(UnPackageUtils& unpackageUtil) = 0;
#endif

};

}
