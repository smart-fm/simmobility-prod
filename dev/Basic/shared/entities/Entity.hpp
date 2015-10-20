//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <iostream>
#include <queue>
#include <string>
#include <set>
#include <vector>
#include <stdexcept>
#include <sstream>

#include "metrics/Frame.hpp"
#include "util/LangHelpers.hpp"
#include "message/MessageHandler.hpp"
#include "event/EventListener.hpp"


namespace sim_mob
{

class BufferedBase;
class Worker;
class WorkerProvider;
class WorkGroup;
class PartitionManager;

/**
 * Base class of all agents and other "decision-making" entities.
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 * \author Harish Loganathan
 * \author Neeraj Deshmukh
 */
class Entity : public messaging::MessageHandler, public event::EventListener
{
private:

protected:

	/** The entity id */
	unsigned int id;

	/** The start-time of the entity (milliseconds) */
	unsigned int startTime;

	/** indicator for entity which has to update multiple times in a time tick */
	bool multiUpdate;

	/**
	 * Handles all messages sent to the MessageHandler implementation.
	 *
	 * @param type of the message.
	 * @param message data received.
	 */
	virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message)
	{
	}

	/**
	 * Builds the list of Buffered<> types this entity subscribes to. Any subclass of Entity should
	 * override this method. The first thing to do is call the buildSubscriptionList() method of the immediate
	 * base class, which will construct the subscription list up to this point. Then, any Buffered types in the
	 * current class should be added to subscriptionList_cached.
	 *
	 * @return the list of Buffered<> types this entity subscribes to
	 */
	virtual std::vector<sim_mob::BufferedBase*> buildSubscriptionList() = 0;

	/**Callback method called when this entity enters (migrates in) into a new Worker.*/
	virtual void onWorkerEnter();

	/**Callback method called when this entity exits (migrates out) from the current Worker.*/
	virtual void onWorkerExit();

public:

	/**The parent entity for this entity.*/
	Entity* parentEntity;

	/**
	 * Pointer to the worker provider object which is currently managing this Entity.
	 * NOTE: Do *not* replace this with a direct pointer to the Worker; it is too dangerous.
	 */
	WorkerProvider* currWorkerProvider;

	/**Indicates if the entity is a fake entity (Only used when executing in MPI mode)*/
	bool isFake;

	/**
	 * Indicates if the entity received from another machine is a duplicate fake entity
	 * (Only used when executing in MPI mode)
	 */
	bool isDuplicateFakeEntity;

	/**Value returned from update() and used to signal changes to the parent Worker class.*/
	struct UpdateStatus
	{
		/**Return status of the update() function.*/
		enum RET_STATUS
		{
			/**Continue processing next time tick.*/
			RS_CONTINUE,

			/**More processing required in same time tick*/
			RS_CONTINUE_INCOMPLETE,

			/**Done, remove from the simulation*/
			RS_DONE
		};

		/**Helper variable: represents a simple "Done" state with no changed variables.*/
		static const UpdateStatus Continue;
		static const UpdateStatus ContinueIncomplete;
		static const UpdateStatus Done;

		/**
		 * Note: Construction requires at least the return code.
		 * More complex construction takes two vectors of variables and extracts which ones are old/new.
		 */
		explicit UpdateStatus(RET_STATUS status, const std::vector<BufferedBase*>& currTickVals = std::vector<BufferedBase*>(),
							const std::vector<BufferedBase*>& nextTickVals = std::vector<BufferedBase*>());

		/**The return status*/
		RET_STATUS status;

		/**BufferedBase* items to remove from the parent worker after this time tick.*/
		std::set<BufferedBase*> toRemove;

		/**BufferedBase* items to add to the parent worker for the next time tick.*/
		std::set<BufferedBase*> toAdd;
	};

	explicit Entity(unsigned int id);
	virtual ~Entity();

	virtual void setStartTime(unsigned int value)
	{
		startTime = value;
	}

	virtual unsigned int getStartTime() const
	{
		return startTime;
	}

	unsigned int getId() const
	{
		return id;
	}

	bool isMultiUpdate() const
	{
		return multiUpdate;
	}

	/**
	 * Update function. This will be called each time tick (at the entity type's granularity),
	 * and will update the entity's state. During this phase,
	 * the entity may call any Buffered data type's "get" method, but may only "set" its own
	 * Buffered data. Flip is called after each update phase.
	 *
	 * @param now The timeslice representing the time frame for which this method is called
	 *
	 * @return The return value is specified by the UpdateStatus class, which allows for an Entity to "Continue"
	 * processing or be considered "Done" processing, and optionally to manually register and deregister
	 * several buffered types.
	 * If this function returns Done, the Entity should be considered finished with its work
	 * and may be removed from the Simulation and deleted. Buffered types should all be considered
	 * moot at this point.
	 */
	virtual UpdateStatus update(timeslice now) = 0;

	/**
	 * Used to determine if an entity is spatial in nature. A non-spatial Agent cannot be identified by its location,
	 * which will usually be (0,0) (but this is not guaranteed).
	 * Subclasses should override this function to indicate that they should not be considered as part of the spatial index.
	 * Note that they *may* still have a geo-spatial location (e.g., Signals), but this location should not be searchable.
	 *
	 * @return true if the Agent is "non-spatial" in nature.
	 *
	 * Note: if true, it should be added to our spatial index.
	 */
	virtual bool isNonspatial() = 0;

	/**
	 * registers child entity
	 */
	virtual void registerChild(Entity* child = nullptr);

	/**
	 * Inform parent to cut off connection with it if necessary
	 *
	 * @param child The child entity to be un-registered
	 */
	virtual void unregisterChild(Entity* child = nullptr);

	/**
	 * Returns a list of pointers to each Buffered<> type that this entity managed.
	 * Entity sub-classes should override buildSubscriptionList() to help with
	 * this process.
	 */
	std::vector<BufferedBase*> getSubscriptionList();

	/**
	 * Only the WorkGroup can retrieve/set the currWorkerProvider flag. I'm doing this through a
	 * friend class, since get/set methods have the potential for abuse (currWorker can't be declared const*)
	 */
	friend class Worker;
	friend class WorkerGroup;
	friend class PartitionManager;
};


/**Comparison for the priority queue*/
struct cmp_agent_start : public std::less<Entity*>
{
	bool operator()(const Entity* x, const Entity* y) const;
};

/**C++ static constructors*/
class StartTimePriorityQueue : public std::priority_queue<Entity*, std::vector<Entity*>, cmp_agent_start>
{
};
}

