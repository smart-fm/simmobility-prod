//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <set>
#include <queue>
#include <vector>
#include <functional>
#include <stdexcept>

//These are minimal header file, so please keep includes to a minimum.
#include "conf/settings/DisableOutput.h"
#include "conf/settings/DisableMPI.h"

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/random.hpp>

#include "buffering/Shared.hpp"
#include "entities/Entity.hpp"
#include "entities/PendingEvent.hpp"
#include "logging/NullableOutputStream.hpp"
#include "event/EventListener.hpp"

namespace sim_mob
{
	namespace long_term
	{

		class Agent_LT;
		class PackageUtils;
		class UnPackageUtils;


		//It is not a good design, now. Need to verify.
		//The class is used in Sim-Tree for Bottom-Up Query
		struct TreeItem;

		//Comparison for our priority queue
		struct cmp_agent_lt_start : public std::less<Agent_LT*>
		{
		  bool operator() (const Agent_LT* x, const Agent_LT* y) const;
		};

		struct cmp_event_lt_start : public std::less<PendingEvent>
		{
		  bool operator() (const PendingEvent& x, const PendingEvent& y) const;
		};

		//C++ static constructors...
		class StartTimePriorityQueue_lt : public std::priority_queue<Agent_LT*, std::vector<Agent_LT*>, cmp_agent_lt_start> {};
		class EventTimePriorityQueue_lt : public std::priority_queue<PendingEvent, std::vector<PendingEvent>, cmp_event_lt_start> {};

		/**
		 * Basic Agent class.
		 *	//Internal update function for handling frame_init(), frame_tick(), etc.
			sim_mob::Entity::UpdateStatus perform_update(timeslice now);
		 *
		 * \author Seth N. Hetu
		 * \author Luo Linbo
		 * \author LIM Fung Chai
		 * \author Wang Xinyuan
		 * \author Xu Yan
		 * \author zhang huai peng
		 * \author chetan rogbeer <chetan.rogbeer@smart.mit.edu>
		 */
		class Agent_LT : public sim_mob::Entity
		{
		public:
			///Construct an Agent with an immutable ID.
			///Note that, if -1, the Agent's ID will be assigned automatically. This is the preferred
			///  way of dealing with agent ids. In the case of explicit ID assignment (anything >=0),
			///  the Agent class will take NO measures to ensure that no Agents share IDs.
			explicit Agent_LT(const MutexStrategy& mtxStrat, int id=-1);
			virtual ~Agent_LT();

			///Update agent behavior. This will call frame_init, frame_tick, etc. correctly.
			///Sub-classes should generally override the frame_* methods, but overriding update()
			/// is ok too (it just overrides all the safeguards).
			virtual Entity::UpdateStatus update(timeslice now);

			///A temporary list of configuration properties used to load an Agent's role from the config file.
			void setConfigProperties(const std::map<std::string, std::string>& props);

			const std::map<std::string, std::string>& getConfigProperties();

			void clearConfigProperties();

			//Removal methods
			bool isToBeRemoved();
			void setToBeRemoved();
			void clearToBeRemoved(); ///<Temporary function.

			/**
			 * Inherited from MessageHandler.
			 */
			 virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message) = 0;

			///Retrieve a monotonically-increasing unique ID value.
			///\param preferredID Will be returned if it is greater than the current maximum-assigned ID.
			///\note
			///Passing in a negative number will always auto-assign an ID, and is recommended.
			static unsigned int GetAndIncrementID(int preferredID);

			void buildSubscriptionList(std::vector<sim_mob::BufferedBase*>& subsList);

			///Set the start ID for automatically generated IDs.
			///\param startID Must be >0
			///\param failIfAlreadyUsed If true, fails with an exception if the auto ID has already been used or set.
			static void SetIncrementIDStartValue(int startID, bool failIfAlreadyUsed);

			const sim_mob::MutexStrategy& getMutexStrategy()
			{
				return mutexStrat;
			}

			int getOwnRandomNumber();

			bool isInitialized() const
			{
				return initialized;
			}

			void setInitialized(bool init)
			{
				initialized = init;
			}

			long getLastUpdatedFrame();

			void setLastUpdatedFrame(long lastUpdatedFrame);

			bool isNonspatial();

			/**
			 * xuyan: All Agents should have the serialization functions implemented for Distributed Version
			 */
		#ifndef SIMMOB_DISABLE_MPI
		public:
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

			timeslice currTick;// curr Time tick

			///Agents can access all other agents (although they usually do not access by ID)
			static std::set<Entity*> all_agents;
			static StartTimePriorityQueue_lt pending_agents; //Agents waiting to be added to the simulation, prioritized by start time.

			static int createdAgents;
			static int diedAgents;

		protected:
			///Raises an exception if the given Agent was started either too early or too late, or exists past its end time.
			static void CheckFrameTimes(unsigned int agentId, uint32_t now, unsigned int startTime, bool wasFirstFrame, bool wasRemoved);

			///Access the Logger.
			///Note that the non-standard capitalization of this function is left in for compatibility with its previous usage as a class.
			sim_mob::NullableOutputStream Log();

			///Called during the first call to update() for a given agent.
			///Return false to indicate failure; the Agent will be removed from the simulation with no
			/// further processing.
			virtual bool frame_init(timeslice now);

			///Called during every call to update() for a given agent. This is called after frame_tick()
			/// for the first call to update(). Return an UpdateStatus indicating future action.
			///NOTE: Returning "UpdateStatus::RS_DONE" will negate the final line of output.
			//       It is generally better to call "setToBeRemoved()", and return "UpdateStatus::RS_CONTINUE",
			//       even though conceptually this is slightly confusing. We can clean this up later.
			virtual Entity::UpdateStatus frame_tick(timeslice now);

			///Called after frame_tick() for every call to update() for a given agent.
			///Use this method to display output for this time tick.
			virtual void frame_output(timeslice now);
			/**
			 * Handler for frame_init method from agent.
			 * @param now time.
			 * @return true if the init ran well or false otherwise.
			 */
			 virtual bool onFrameInit(timeslice now) = 0;

			/**
			 * Handler for frame_tick method from agent.
			 *
			 * @param now time.
			 * @return update status.
			 */
			virtual sim_mob::Entity::UpdateStatus onFrameTick(timeslice now) = 0;

			/**
			 * Handler for frame_output method from agent.
			 * @param now time.
			 */
			virtual void onFrameOutput(timeslice now) = 0;

			///TODO: Temporary; this allows a child class to reset "call_frame_init", but there is
			///      probably a better way of doing it.
			void resetFrameInit();

			int dynamic_seed;

		private:
			//Internal update function for handling frame_init(), frame_tick(), etc.
			sim_mob::Entity::UpdateStatus perform_update(timeslice now);

			//For future reference.
			const sim_mob::MutexStrategy mutexStrat;

			bool toRemoved;
			static unsigned int next_agent_id;

			///Should this agent call frame_init()?
			///NOTE: This only applies to the Agent; a Person, for example, may call frame_init()
			///      on its Roles from its own frame_tick() method.
			bool initialized;

			//Unknown until runtime
			std::map<std::string, std::string> configProperties;

			long lastUpdatedFrame; //Frame number in which the previous update of this agent took place
		};
	}
}








