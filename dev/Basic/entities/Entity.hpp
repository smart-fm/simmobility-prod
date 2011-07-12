#pragma once

#include "../frame.hpp"
#include "../buffering/BufferedDataManager.hpp"


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
	}

	/**
	 * Update function. This will be called each time tick (at the entity type's granularity; see
	 * simpleconf.hpp for more information), and will update the entity's state. During this phase,
	 * the entity may call any Buffered data type's "get" method, but may only "set" its own
	 * Buffered data. Flip is called after each update phase.
	 */
	virtual void update(frame_t frameNumber) = 0;

	/**
	 * Returns a list of pointers to each Buffered<> type that this entity managed.
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
	 * Build the list of Buffered<> types this entity subscribes to. Used by sub-classes
	 */
	virtual void buildSubscriptionList() = 0;

private:
	unsigned int id;
	bool isSubscriptionListBuilt;

protected:
	std::vector<sim_mob::BufferedBase*> subscriptionList_cached;

//Trivial accessors/mutators. Header-implemented
public:
	unsigned int getId() const { return id; }
};


}

