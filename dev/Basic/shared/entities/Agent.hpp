//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/random.hpp>
#include <functional>
#include <set>
#include <stdexcept>
#include <vector>

//These are minimal header file, so please keep includes to a minimum.
#include "conf/settings/DisableOutput.h"
#include "conf/settings/DisableMPI.h"

#include "buffering/Shared.hpp"
#include "entities/Entity.hpp"
#include "entities/TravelTimeManager.hpp"
#include "event/EventListener.hpp"
#include "geospatial/RoadRunnerRegion.hpp"
#include "logging/NullableOutputStream.hpp"

namespace sim_mob
{

class Lane;
class Link;
class WorkGroup;
class BufferedBase;
class ShortTermBoundaryProcessor;
class PackageUtils;
class UnPackageUtils;
class RoadSegment;
class RoadRunnerRegion;

//It is not a good design, now. Need to verify.
//The class is used in Sim-Tree for Bottom-Up Query
struct TreeItem;

/**
 * Basic Agent class.
 *
 * \author Seth N. Hetu
 * \author Luo Linbo
 * \author LIM Fung Chai
 * \author Wang Xinyuan
 * \author Xu Yan
 * \author zhang huai peng
 * \author Harish Loganathan
 * \author Neeraj Deshmukh
 */
class Agent : public sim_mob::Entity
{
private:
	/**The mutex strategy for the agent*/
	const sim_mob::MutexStrategy mutexStrat;

	/**Keeps track of the next agent's id. Used for auto-generating the agent id's*/
	static unsigned int nextAgentId;

	/**Indicates if the agent has been initialised using the frame_init method*/
	bool initialized;

	/**
	 * Internal update function for handling frame_init(), frame_tick(), etc.
	 *
	 * @param now The timeslice representing the time frame for which this method is called
	 *
	 * @return the status of the update method
	 */
	sim_mob::Entity::UpdateStatus performUpdate(timeslice now);

	/**
	 * Checks if the update method is being called for the agent before it's start time, later than the
	 * intended start time or after it's end time and raises an exception if so.
	 *
	 * @param agentId The agent's id
	 * @param now The current time
	 * @param startTime The agent's start time
	 * @param wasFirstFrame Indicates if the frame_init() method was called for the agent
	 * @param wasRemoved Indicates if the agent was removed from the simulation
	 */
	static void checkFrameTimes(unsigned int agentId, uint32_t now, unsigned int startTime, bool wasFirstFrame, bool wasRemoved);

protected:
	/**Indicates if the agent is to be removed from the simulation*/
	bool toRemoved;

	/**Stores the frame number in which the previous update of this agent took place*/
	long lastUpdatedFrame;
	
	/**
	 * local seed for random number generator (initialized to agent id directly passed into the constructor)
	 *
	 * NOTE: this seed seems to be used only in MPI code.
	 * TODO: revisit this when testing MPI
	 */
	int dynamicSeed;

	/**
	 * Access the Logger.
	 * Note that the non-standard capitalisation of this function is left in for compatibility with its previous usage as a class.
	 *
	 * @return an output stream for the current worker
	 */
	sim_mob::NullableOutputStream Log() const;

	/**
	 * Called during the first call to update() for a given agent.
	 *
	 * @param now The timeslice representing the time frame for which this method is called
	 *
	 * @return UpdateStatus with the value of 'status' set to 'RS_CONTINUE', 'RS_CONTINUE_INCOMPLETE' or 'RS_DONE' to indicate the 
	 * result of the execution.
	 */
	virtual UpdateStatus frame_init(timeslice now) = 0;

	/**
	 * Called during every call to update() for a given agent. This is called after frame_tick()
	 * for the first call to update().
	 * NOTE: Returning "UpdateStatus::RS_DONE" will negate the final line of output.
	 * It is generally better to call "setToBeRemoved()", and return "UpdateStatus::RS_CONTINUE",
	 * even though conceptually this is slightly confusing. We can clean this up later.
	 *
	 * @param now The timeslice representing the time frame for which this method is called
	 *
	 * @return an UpdateStatus indicating future action
	 */
	virtual Entity::UpdateStatus frame_tick(timeslice now) = 0;

	/**
	 * Called after frame_tick() for every call to update() for a given agent.
	 * Use this method to display output for this time tick.
	 *
	 * @param now The timeslice representing the time frame for which this method is called
	 */
	virtual void frame_output(timeslice now) = 0;

	/**
	 * Sets the initialised flag to false, enabling frame_init() to be called again
	 */
	void unsetInitialized()
	{
		initialized = false;
	}

public:
	/**The agent's position (X-coordinate)*/
	sim_mob::Shared<int> xPos;

	/**The agent's position (Y-coordinate)*/
	sim_mob::Shared<int> yPos;

	/**The current time tick*/
	timeslice currTick;

	/**The set of all agents. Agents can access all other agents (although they usually do not access by ID)*/
	static std::set<Entity*> all_agents;

	/**Agents waiting to be added to the simulation, prioritised by start time.*/
	static StartTimePriorityQueue pending_agents;
	static std::map<std::string,Entity*> activeAgents;

	/**
	 * Constructs an Agent with an immutable ID.
	 *
	 * @param mtxStrat Mutex strategy for shared members
	 * @param id If -1, the Agent's ID will be assigned automatically. This is the preferred
	 * way of dealing with agent ids. In the case of explicit ID assignment (anything >=0),
	 * the Agent class will take NO measures to ensure that no Agents share IDs.
	 */
	explicit Agent(const MutexStrategy& mtxStrat, int id = -1);
	virtual ~Agent();

	/**
	 * The agent update behaviour. This will call frame_init(), frame_tick(), etc. correctly.
	 * Sub-classes should generally override the frame_* methods, but overriding update()
	 * is ok too (it just overrides all the safeguards).
	 *
	 * @param now The timeslice representing the time frame for which this method is called
	 *
	 * @return status after update
	 */
	virtual Entity::UpdateStatus update(timeslice now);

	/**
	 * Builds a subscriptions list to be added to the managed data of the parent worker
	 *
	 * @return the list of Buffered<> types this entity subscribes to
	 */
	virtual std::vector<BufferedBase *> buildSubscriptionList();

	/**
	 * Checks whether the agent is earmarked to be removed from the simulation
	 *
	 * @return true, if the agent is about to leave the simulation
	 */
	bool isToBeRemoved();

	/**
	 * Marks the agent for removal from the simulation
	 */
	void setToBeRemoved();

	/**
	 * Inherited from EventListener.
	 *
	 * @param eventId
	 * @param ctxId
	 * @param sender
	 * @param args
	 */
	virtual void onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);

	/**
	 * Inherited from MessageHandler.
	 *
	 * @param type
	 * @param message
	 */
	virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);

	/**
	 * Retrieves a monotonically-increasing unique ID value. Passing in a negative number will
	 * always auto-assign an ID, and is recommended.
	 *
	 * @param preferredID preferredID Will be returned if it is greater than the current maximum-assigned ID.
	 * @return a unique ID value
	 */
	static unsigned int getAndIncrementID(int preferredID);

	/**
	 * Set the start ID for automatically generated IDs.
	 *
	 * @param startID The agent's start ID. This must be > 0
	 * @param failIfAlreadyUsed If true, fails with an exception if the auto ID has already been used or set.
	 */
	static void setIncrementIDStartValue(int startID, bool failIfAlreadyUsed);

	const sim_mob::MutexStrategy& getMutexStrategy()
	{
		return mutexStrat;
	}

	bool isInitialized() const
	{
		return initialized;
	}

	void setInitialized(bool init)
	{
		initialized = init;
	}

	long getLastUpdatedFrame() const
	{
		return lastUpdatedFrame;
	}

	void setLastUpdatedFrame(long lastUpdatedFrame)
	{
		this->lastUpdatedFrame = lastUpdatedFrame;
	}

	/**
	 * This struct is used to track the Regions and Paths available to this Agent. This functionality is
	 * ONLY used in RoadRunner, so putting it in Agent is not ideal. At the moment, I am not sure of the best
	 * way to resolve this (as the commsim code uses a generic "Agent" in most cases), so I'm putting it in
	 * the most obvious place. Ideally, we would have a framework for "optional" elements such as this.
	 * Note that, if the RegionAndPathTracker is disabled, attempting to call getNewRegion/PathSet() will throw
	 * an exception.
	 */
	struct RegionAndPathTracker
	{
	private:
		std::vector<sim_mob::RoadRunnerRegion> newAllRegions;
		std::vector<sim_mob::RoadRunnerRegion> newRegionPath;
		bool enabled;

	public:
		RegionAndPathTracker() : enabled(false)
		{
		}

		/**Enable Region tracking. Without this, the "get()" functions will throw.*/
		void enable()
		{
			enabled = true;
		}

		bool isEnabled() const
		{
			return enabled;
		}

		void resetAllRegionsSet()
		{
			newAllRegions.clear();
		}

		void resetNewRegionPath()
		{
			newRegionPath.clear();
		}

		std::vector<sim_mob::RoadRunnerRegion> getNewAllRegionsSet() const
		{
			if (!enabled)
			{
				throw std::runtime_error("Agent Region Tracking is disabled.");
			}
			return newAllRegions;
		}

		std::vector<sim_mob::RoadRunnerRegion> getNewRegionPath() const
		{
			if (!enabled)
			{
				throw std::runtime_error("Agent Region Tracking is disabled.");
			}
			return newRegionPath;
		}

		void setNewAllRegionsSet(const std::vector<sim_mob::RoadRunnerRegion>& value)
		{
			newAllRegions = value;
		}

		void setNewRegionPath(const std::vector<sim_mob::RoadRunnerRegion>& value)
		{
			newRegionPath = value;
		}

	} regionAndPathTracker;

	/**
     * @return the current set of "all Regions", but only if region-tracking is enabled, and only if
	 * the region set has changed since the last time tick.
     */
	std::vector<sim_mob::RoadRunnerRegion> getAndClearNewAllRegionsSet()
	{
		std::vector<sim_mob::RoadRunnerRegion> res = regionAndPathTracker.getNewAllRegionsSet();
		regionAndPathTracker.resetAllRegionsSet();
		return res;
	}

	/**
     * @return the current set of Regions this Agent expects to travel through on its way to its goal,
	 * but only if region-tracking is enabled, and only if the path set has changed since the last time tick.
     */
	std::vector<sim_mob::RoadRunnerRegion> getAndClearNewRegionPath()
	{
		std::vector<sim_mob::RoadRunnerRegion> res = regionAndPathTracker.getNewRegionPath();
		regionAndPathTracker.resetNewRegionPath();
		return res;
	}

	RegionAndPathTracker& getRegionSupportStruct()
	{
		return regionAndPathTracker;
	}

	const RegionAndPathTracker& getRegionSupportStruct() const
	{
		return regionAndPathTracker;
	}

#ifndef SIMMOB_DISABLE_MPI
	/**
	 * xuyan: All Agents should have the serialization functions implemented for Distributed Version
	 */
	friend class BoundaryProcessor;
	friend class ShortTermBoundaryProcessor;

	/**
	 * Used for crossing agents
	 */
	virtual void pack(PackageUtils& packageUtil);
	virtual void unpack(UnPackageUtils& unpackageUtil);

	/**
	 * used for feedback and feed forward agents
	 */
	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif

};

}
