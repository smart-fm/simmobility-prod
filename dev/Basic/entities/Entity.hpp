/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <string>
#include <stdexcept>
#include <sstream>

#include "metrics/Frame.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "workers/Worker.hpp"


namespace sim_mob
{

class WorkGroup;


/**
 * Base class of all agents and other "decision-making" entities.
 */
class Entity {
public:
	///Construct an entity with an immutable ID
	Entity(unsigned int id) : id(id), isSubscriptionListBuilt(false), currWorker(nullptr) {}
	virtual ~Entity() {
		if (currWorker) {
			//Note: If a worker thread is still active for this agent, that's a major problem. But
			//      we can't throw an exception since that may lead to a call of terminate().
			//      So we'll output a message and terminate manually, since throwing exceptions from
			//      a destructor is iffy at best.
			std::cout <<"Error: Deleting an Entity which is still being managed by a Worker." <<std::endl;
			abort();
		}
	}


	/**
	 * Update function. This will be called each time tick (at the entity type's granularity; see
	 * simpleconf.hpp for more information), and will update the entity's state. During this phase,
	 * the entity may call any Buffered data type's "get" method, but may only "set" its own
	 * Buffered data. Flip is called after each update phase.
	 */
	virtual void update(frame_t frameNumber) = 0;


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

protected:
	///Who is currently managing this Entity?
	Worker<Entity>* currWorker;

	//Only the WorkGroup can retrieve/set the currWorker flag. I'm doing this through a
	// friend class, since get/set methods have the potential for abuse (currWorker can't be declared const*)
	friend class WorkGroup;

//Some near-trivial functions
public:
	///Retrieve this Entity's id. An entity's ID must be unique within a given machine.
	unsigned int getId() const { return id; }

	///Retrieve this Entity's global id. An entity's Global ID must be unique across machine boundaries,
	///  and is currently made by concatenating the result of getId() to the current machine's IP address.
	///
	///\note
	///This function currently assumes one machine (localhost); it should be modified once our MPI code is
	//   finished.
	std::string getGlobalId() const {
		std::stringstream res;
		res <<"127.0.0.1:" <<id;
		return res.str();
	}


	/**
	 * Returns a list of pointers to each Buffered<> type that this entity managed.
	 * Entity sub-classes should override buildSubscriptionList() to help with
	 * this process.
	 */
	std::vector<sim_mob::BufferedBase*>& getSubscriptionList() {
		if (!isSubscriptionListBuilt) {
			subscriptionList_cached.clear();
			buildSubscriptionList();
			isSubscriptionListBuilt = true;
		}
		return subscriptionList_cached;
	}
};


}

