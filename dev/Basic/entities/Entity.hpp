/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "metrics/Frame.hpp"
#include "buffering/BufferedDataManager.hpp"


namespace sim_mob
{


/**
 * Base class of all agents and other "decision-making" entities.
 */
class Entity {
public:
	///Construct an entity with an immutable ID
	Entity(unsigned int id) : id(id), isSubscriptionListBuilt(false)
	{
		isFake = false;
	}

	/**
	 * Returns a list of pointers to each Buffered<> type that this entity managed.
	 * Entity sub-classes should override buildSubscriptionList() to help with
	 * this process.
	 */
	std::vector<sim_mob::BufferedBase*>& getSubscriptionList()
	{
		if (!isSubscriptionListBuilt) {
			subscriptionList_cached.clear();
			buildSubscriptionList();
			isSubscriptionListBuilt = true;
		}
		return subscriptionList_cached;
	}

	/**
	 * Update function. This will be called each time tick (at the entity type's granularity; see
	 * simpleconf.hpp for more information), and will update the entity's state. During this phase,
	 * the entity may call any Buffered data type's "get" method, but may only "set" its own
	 * Buffered data. Flip is called after each update phase.
	 */
	virtual void update(frame_t frameNumber) = 0;

	virtual void output(frame_t frameNumber) = 0;

protected:
	/**
	 * Build the list of Buffered<> types this entity subscribes to. Any subclass of
	 * Entity should override this method. The first thing to do is call the immediate base
	 * class's buildSubscriptionList() method, which will construct the subscription list up to
	 * this point. Then, any Buffered types in the current class should be added to subscriptionList_cached.
	 */
	virtual void buildSubscriptionList() = 0;
	std::vector<sim_mob::BufferedBase*> subscriptionList_cached;

private:
	unsigned int id;
	bool isSubscriptionListBuilt;

//Trivial accessors/mutators. Header-implemented
public:
	unsigned int getId() const { return id; }

	//add by xuyan
public:
	friend class AgentPackageManager;
	friend class RoadNetworkPackageManager;
	bool isFake;
	bool receiveTheFakeEntityAgain;
};


}

