//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <set>
#include <queue>
#include <vector>
#include <functional>

//TODO: Move to cpp file.
#include <stdexcept>
#include "geospatial/RoadRunnerRegion.hpp"
//END TODO

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

namespace sim_mob {

class Agent;
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

//Comparison for our priority queue
struct cmp_agent_start : public std::less<Agent*> {
  bool operator() (const Agent* x, const Agent* y) const;
};

struct cmp_event_start : public std::less<PendingEvent> {
  bool operator() (const PendingEvent& x, const PendingEvent& y) const;
};

//C++ static constructors...
class StartTimePriorityQueue : public std::priority_queue<Agent*, std::vector<Agent*>, cmp_agent_start> {
};
class EventTimePriorityQueue : public std::priority_queue<PendingEvent, std::vector<PendingEvent>, cmp_event_start> {
};

/**
 * Basic Agent class.
 *
 * \author Seth N. Hetu
 * \author Luo Linbo
 * \author LIM Fung Chai
 * \author Wang Xinyuan
 * \author Xu Yan
 * \author zhang huai peng
 *
 * Agents maintain an x and a y position. They may have different behavioral models.
 */
class Agent : public sim_mob::Entity {
public:
	static int createdAgents;
	static int diedAgents;
	bool errorFlag;
	///Construct an Agent with an immutable ID.
	///Note that, if -1, the Agent's ID will be assigned automatically. This is the preferred
	///  way of dealing with agent ids. In the case of explicit ID assignment (anything >=0),
	///  the Agent class will take NO measures to ensure that no Agents share IDs.
	explicit Agent(const MutexStrategy& mtxStrat, int id=-1);
	virtual ~Agent();

	///Load an agent.
	virtual void load(const std::map<std::string, std::string>& configProps) = 0;

	///Update agent behavior. This will call frame_init, frame_tick, etc. correctly.
	///Sub-classes should generally override the frame_* methods, but overriding update()
	/// is ok too (it just overrides all the safeguards).
	virtual Entity::UpdateStatus update(timeslice now);


protected:
	///Access the Logger.
	///Note that the non-standard capitalization of this function is left in for compatibility with its previous usage as a class.
 	sim_mob::NullableOutputStream Log();


	///Called during the first call to update() for a given agent.
	///Return false to indicate failure; the Agent will be removed from the simulation with no
	/// further processing.
	virtual bool frame_init(timeslice now) = 0;


	///Called during every call to update() for a given agent. This is called after frame_tick()
	/// for the first call to update(). Return an UpdateStatus indicating future action.
	///NOTE: Returning "UpdateStatus::RS_DONE" will negate the final line of output.
	//       It is generally better to call "setToBeRemoved()", and return "UpdateStatus::RS_CONTINUE",
	//       even though conceptually this is slightly confusing. We can clean this up later.
	virtual Entity::UpdateStatus frame_tick(timeslice now) = 0;


	///Called after frame_tick() for every call to update() for a given agent.
	///Use this method to display output for this time tick.
	virtual void frame_output(timeslice now) = 0;


public:

    ///A temporary list of configuration properties used to load an Agent's role from the config file.
    void setConfigProperties(const std::map<std::string, std::string>& props) {
    	this->configProperties = props;
    }
    const std::map<std::string, std::string>& getConfigProperties() {
    	return this->configProperties;
    }
    void clearConfigProperties() {
    	this->configProperties.clear();
    }

	///Subscribe this agent to a data manager.
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

	//Removal methods
	bool isToBeRemoved();
	void setToBeRemoved();
	void clearToBeRemoved(); ///<Temporary function.

	/**
	 * Inherited from EventListener.
	 */
	virtual void onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);

	/**
	 * Inherited from MessageHandler.
	 */
	 virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);

protected:
	///TODO: Temporary; this allows a child class to reset "call_frame_init", but there is
	///      probably a better way of doing it.
	void resetFrameInit();

	//Ask this Agent to re-route.
	virtual void rerouteWithBlacklist(const std::vector<const sim_mob::Link*>& blacklisted);

private:
	//For future reference.
	const sim_mob::MutexStrategy mutexStrat;

	//Internal update function for handling frame_init(), frame_tick(), etc.
	sim_mob::Entity::UpdateStatus perform_update(timeslice now);

public:
	//Either linkTravelStats or RdSegTravelStat will be populated depending on the requirements for link or road segment travel times
	struct linkTravelStats
	{
	public:
		const Link* link_;
		double entryTime;
//		double exitTime;
//		std::map<double, unsigned int> rolesMap; //<timestamp, newRoleID>

		/* note: link exit time will be available only when the agent exits the
		 * link. -1 is a dummy value indication that the agent is in the link*/
		linkTravelStats(const Link* link,
				unsigned int linkEntryTime)
		: link_(link), entryTime(linkEntryTime)//, exitTime(-1)
		{}
	};

	/**
	 * structur to record travel time of agents through a road segment
	 */
	struct RdSegTravelStat
	{
		const RoadSegment* rs;
		double entryTime;
		unsigned int entryTimeInterval;	//unused
		double travelTime;
		std::string travelMode;
		bool started;
		bool finalized;
		std::string dbg;				//todo:debug
		sim_mob::Agent * ag;
		RdSegTravelStat(const RoadSegment* rdSeg, std::string travelMode = "")
		: rs(rdSeg), entryTime(0.0), travelTime(0.0),started(false),finalized(false), travelMode(travelMode), ag(nullptr),entryTimeInterval(0)
		{}

		/**
		 * this method is called when agent enters the segment
		 * @param rdSeg RoadSegment
		 * @rdSegEntryTime RoadSegment Entry time
		 */
		void start(sim_mob::Agent * ag_, const RoadSegment* rdSeg, double &rdSegEntryTime)
		{
			if(started)
			{
				throw std::runtime_error("Starting a travel time which was started before");
			}
			rs = rdSeg;
			entryTime = rdSegEntryTime;
			ag = ag_;
			started = true;
		}

		/**
		 * this method is called when agent exits the segment
		 * @param rdSeg RoadSegment
		 * @rdSegEntryTime RoadSegment Exit time
		 */
		void finalize(const RoadSegment* rdSeg,double &rdSegExitTime, const std::string &travelMode_)
		{
			if(!started)
			{
				throw std::runtime_error("Finalizing a travel time which never started");
			}
			if(finalized)
			{
				throw std::runtime_error("Finalizing a travel time which is already finalized");
			}
			if(rdSeg!= rs)
			{
				throw std::runtime_error("Finalizing a wrong travel time");
			}
			if(rs == nullptr)
			{
				throw std::runtime_error("empty road segment supplied for travel time calculations.");
			}
			finalized = true;
			travelTime = rdSegExitTime - entryTime;
			/*
			 * we set the travel_mode in the end coz in our team, the probability of programmers "calling start() method of this structure before changing a role"
			 * is more than the probability of "calling finalize 'after' changing the role". So we set the travel mode(which is dependent on the role type) at the end.
			 * You may change it later if you wish so.
			 */
			travelMode = travelMode_;
		}

		/**
		 * reset the member variables to their un-initialized values
		 */
		void reset(){
			*this = RdSegTravelStat(nullptr);
		}
	};
	RdSegTravelStat currRdSegTravelStats;
public:
	//The agent's start/end nodes.
	WayPoint originNode;
	WayPoint destNode;

	sim_mob::Shared<int> xPos;  ///<The agent's position, X
	sim_mob::Shared<int> yPos;  ///<The agent's position, Y

	sim_mob::Shared<double> fwdVel;  ///<The agent's velocity, X
	sim_mob::Shared<double> latVel;  ///<The agent's velocity, Y

	sim_mob::Shared<double> xAcc;  ///<The agent's acceleration, X
	sim_mob::Shared<double> yAcc;  ///<The agent's acceleration, Y

	timeslice currTick;// curr Time tick

	///Agents can access all other agents (although they usually do not access by ID)
	static std::set<Entity*> all_agents;
	static StartTimePriorityQueue pending_agents; //Agents waiting to be added to the simulation, prioritized by start time.

	///Retrieve a monotonically-increasing unique ID value.
	///\param preferredID Will be returned if it is greater than the current maximum-assigned ID.
	///\note
	///Passing in a negative number will always auto-assign an ID, and is recommended.
	static unsigned int GetAndIncrementID(int preferredID);

	///Set the start ID for automatically generated IDs.
	///\param startID Must be >0
	///\param failIfAlreadyUsed If true, fails with an exception if the auto ID has already been used or set.
	static void SetIncrementIDStartValue(int startID, bool failIfAlreadyUsed);

	///Note: Calling this function from another Agent is extremely dangerous if you
	/// don't know what you're doing.
	boost::mt19937& getGenerator() {
		return gen;
	}

	const sim_mob::MutexStrategy& getMutexStrategy() {
		return mutexStrat;
	}

	void setOnActivity(bool value) { onActivity = value; }
	bool getOnActivity() { return onActivity; }

	void setNextPathPlanned(bool value) { nextPathPlanned = value; }
	bool getNextPathPlanned() { return nextPathPlanned; }

	void setNextEvent(PendingEvent* value) { nextEvent = value; }
	PendingEvent* getNextEvent() { return nextEvent; }

	void setCurrEvent(PendingEvent* value) { currEvent = value; }
	PendingEvent* getCurrEvent() { return currEvent; }

	//==================== road segment travel time computation ===================================
	/**
	 * mark the start of the travelStats for the current road segment
	 */
	RdSegTravelStat &startCurrRdSegTravelStat(const RoadSegment* rdSeg, double entryTime);
	/**
	 * mark the end of the travelStats for each the current road segment
	 */
	RdSegTravelStat &finalizeCurrRdSegTravelStat(const RoadSegment* rdSeg, double exitTime, const std::string &travel_mode);
	RdSegTravelStat &getCurrRdSegTravelStats()
	{
		return currRdSegTravelStats;
	}

//	const std::map<double, RdSegTravelStat>& getRdSegTravelStatsMap()
//	{
//		return this->rdSegTravelStatsMap.get();
//	}
//	sim_mob::Shared< std::map<double, RdSegTravelStat> > rdSegTravelStatsMap; //<linkExitTime, travelStats>
	//==================== end of road segment travel time computation ============================

	//============================ link travel time computation ===================================
	//travelStats for each agent will be updated either for a role change or link change
	void initLinkTravelStats(const Link* link, double entryTime);
	void addToLinkTravelStatsMap(linkTravelStats ts, double exitTime);
	linkTravelStats getLinkTravelStats()
	{
		return currLinkTravelStats;
	}

	const std::map<double, linkTravelStats>& getLinkTravelStatsMap()
	{
		return this->linkTravelStatsMap.get();
	}

	linkTravelStats currLinkTravelStats;
	sim_mob::Shared< std::map<double, linkTravelStats> > linkTravelStatsMap; //<linkExitTime, travelStats>
	//=============================end of link travel time computation ===========================

	bool isQueuing;
	double distanceToEndOfSegment;
	double drivingTimeToEndOfLink;
	double movingVelocity;

	//timeslice enqueueTick;

protected:
	///Raises an exception if the given Agent was started either too early or too late, or exists past its end time.
	static void CheckFrameTimes(unsigned int agentId, uint32_t now, unsigned int startTime, bool wasFirstFrame, bool wasRemoved);

private:
	//unsigned int currMode;
	bool toRemoved;
	static unsigned int next_agent_id;

	///Should this agent call frame_init()?
	///NOTE: This only applies to the Agent; a Person, for example, may call frame_init()
	///      on its Roles from its own frame_tick() method.
	bool initialized;

    //Unknown until runtime
    std::map<std::string, std::string> configProperties;

	PendingEvent* currEvent;
	PendingEvent* nextEvent;

	bool nextPathPlanned; //determines if the detailed path for the current subtrip is already planned

	bool onActivity; //Determines if the person is conducting any activity
	long lastUpdatedFrame; //Frame number in which the previous update of this agent took place

protected:
	int dynamic_seed;

	//Random number generator
	//TODO: For now (for thread safety) I am giving each Agent control over its own random
	//      number stream. We can probably raise this to the Worker level if we require it.
	boost::mt19937 gen;

public:
	int getOwnRandomNumber();


	bool isInitialized() const {
		return initialized;
	}

	void setInitialized(bool init) {
		initialized = init;
	}

	long getLastUpdatedFrame();

	void setLastUpdatedFrame(long lastUpdatedFrame);

	friend class BoundaryProcessor;
	friend class ShortTermBoundaryProcessor;

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


private:
	///Have we registered to receive commsim-related messages?
	bool commEventRegistered;


public:
	/**
	 * This struct is used to track the Regions and Paths available to this Agent. This functionality is
	 *   ONLY used in RoadRunner, so putting it in Agent is not idea. At the moment, I am not sure of the best
	 *   way to resolve this (as the commsim code uses a generic "Agent" in most cases), so I'm putting it in
	 *   the most obvious place. Ideally, we would have a framework for "optional" elements such as this.
	 * Note that, if the RegionAndPathTracker is disabled, attempting to call getNewRegion/PathSet() will throw
	 *   an exception.
	 */
	struct RegionAndPathTracker {
		RegionAndPathTracker() : enabled(false) {}

		///Enable Region tracking. Without this, the "get()" functions will throw.
		void enable() {
			enabled = true;
		}

		///Is Region tracking enabled?
		bool isEnabled() const {
			return enabled;
		}

		///Reset the list of Regions (or paths)
		void resetAllRegionsSet() {
			newAllRegions.clear();
		}
		void resetNewRegionPath() {
			newRegionPath.clear();
		}

		///See: Agent::getNewAllRegionsSet()
		std::vector<sim_mob::RoadRunnerRegion> getNewAllRegionsSet() const {
			if (!enabled) { throw std::runtime_error("Agent Region Tracking is disabled."); }
			return newAllRegions;
		}

		///Set Agent::getNewRegionPath()
		std::vector<sim_mob::RoadRunnerRegion> getNewRegionPath() const {
			if (!enabled) { throw std::runtime_error("Agent Region Tracking is disabled."); }
			return newRegionPath;
		}

		///Set the "all regions" return value.
		void setNewAllRegionsSet(const std::vector<sim_mob::RoadRunnerRegion>& value) {
			newAllRegions = value;
		}

		///Set the "region path" return value.
		void setNewRegionPath(const std::vector<sim_mob::RoadRunnerRegion>& value) {
			newRegionPath = value;
		}

	private:
		///Actual storage + enabled.
		std::vector<sim_mob::RoadRunnerRegion> newAllRegions;
		std::vector<sim_mob::RoadRunnerRegion> newRegionPath;
		bool enabled;
	} regionAndPathTracker;

private:
	///Enable Region support.
	///See RegionAndPathTracker for more information.
	void enableRegionSupport() { regionAndPathTracker.enable(); }

public:
	///Returns the current set of "all Regions", but only if region-tracking is enabled, and only if
	/// the region set has changed since the last time tick.
	///See RegionAndPathTracker for more information.
	std::vector<sim_mob::RoadRunnerRegion> getAndClearNewAllRegionsSet() {
		std::vector<sim_mob::RoadRunnerRegion> res = regionAndPathTracker.getNewAllRegionsSet();
		regionAndPathTracker.resetAllRegionsSet();
		return res;
	}

	///Returns the current set of Regions this Agent expects to travel through on its way to its goal,
	///but only if region-tracking is enabled, and only if the path set has changed since the last time tick.
	///See RegionAndPathTracker for more information.
	std::vector<sim_mob::RoadRunnerRegion> getAndClearNewRegionPath() {
		std::vector<sim_mob::RoadRunnerRegion> res = regionAndPathTracker.getNewRegionPath();
		regionAndPathTracker.resetNewRegionPath();
		return res;
	}

	///Get the Region-support object. This is used for all other Region-related queries.
	RegionAndPathTracker& getRegionSupportStruct() { return regionAndPathTracker; }
	const RegionAndPathTracker& getRegionSupportStruct() const { return regionAndPathTracker; }

};

} //End namespace sim_mob








