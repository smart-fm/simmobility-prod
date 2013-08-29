//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <queue>
#include <vector>
#include <functional>

//These are minimal header file, so please keep includes to a minimum.
#include "conf/settings/DisableOutput.h"
#include "conf/settings/DisableMPI.h"

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/random.hpp>

#include "buffering/Shared.hpp"
#include "entities/Entity.hpp"
#include "entities/PendingEvent.hpp"
#include "event/args/EventArgs.hpp"
#include "event/EventPublisher.hpp"
#include "logging/NullableOutputStream.hpp"


namespace sim_mob {

class Agent;
class Lane;
class Link;
class WorkGroup;
class BufferedBase;
class ShortTermBoundaryProcessor;
class PackageUtils;
class UnPackageUtils;
class ProfileBuilder;

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


DECLARE_CUSTOM_CALLBACK_TYPE (AgentLifeEventArgs)
class AgentLifeEventArgs: public event::EventArgs {
public:
	AgentLifeEventArgs(Agent* agent);
	AgentLifeEventArgs(const AgentLifeEventArgs& orig);
	virtual ~AgentLifeEventArgs();

	/**
	 * Gets the unit affected by the action.
	 * @return
	 */
	const Agent* GetAgent() const;
private:
	Agent* agent;
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
class Agent : public sim_mob::Entity, public event::EventPublisher, public event::EventListener/*, public sim_mob::CommunicationSupport*/ {
public:
	enum AgentLifecycleEvents {
		AGENT_LIFE_EVENT_STARTED_ID = 3000,
		AGENT_LIFE_EVENT_FINISHED_ID = 3001,
		AGENT_INCIDENT_EVENT_ID = 30002,
	};

	static int createdAgents;
	static int diedAgents;

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

	/* *
	 * I'm keeping getters and setters for current lane, segment and link in Agent class to be able to determine the
	 * location of the agent without having to dynamic_cast to Person and get the role.
	 * If these are irrelevant for some sub-class of agent (E.g. Signal), the sub class can just ignore these.
	 * ~ Harish
	 */
	virtual const sim_mob::Link* getCurrLink() const;
	virtual	void setCurrLink(const sim_mob::Link* link);
	virtual const sim_mob::RoadSegment* getCurrSegment() const;
	virtual	void setCurrSegment(const sim_mob::RoadSegment* rdSeg);
	virtual const sim_mob::Lane* getCurrLane() const;
	virtual	void setCurrLane(const sim_mob::Lane* lane);

	virtual void OnEvent(event::EventId eventId, event::EventPublisher* sender, const event::EventArgs& args);

protected:
	///TODO: Temporary; this allows a child class to reset "call_frame_init", but there is
	///      probably a better way of doing it.
	void resetFrameInit();

private:
	//For future reference.
	const sim_mob::MutexStrategy mutexStrat;

	//Internal update function for handling frame_init(), frame_tick(), etc.
	sim_mob::Entity::UpdateStatus perform_update(timeslice now);

public:
	//for mid-term link travel time computation
	struct travelStats
	{
	public:
		const Link* link_;
		unsigned int linkEntryTime_;
		std::map<double, unsigned int> rolesMap; //<timestamp, newRoleID>

		travelStats(const Link* link,
				unsigned int linkEntryTime)
		: link_(link), linkEntryTime_(linkEntryTime)
		{
		}
	};


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

	///Agents can access all other agents (although they usually do not access by ID)
	static std::vector<Entity*> all_agents;
	static StartTimePriorityQueue pending_agents; //Agents waiting to be added to the simulation, prioritized by start time.

	static std::vector<Entity*> agents_on_event; //Agents are conducting event
	static EventTimePriorityQueue agents_with_pending_event; //Agents with upcoming event, prioritized by start time.


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

	//used for mid-term supply for link travel time computation
	//travelStats for each agent will be updated either for a role change or link change
	void initTravelStats(const Link* link, double entryTime);
	void addToTravelStatsMap(travelStats ts, double exitTime);
	travelStats getTravelStats()
	{
		return currTravelStats;
	}

	const std::map<double, travelStats>& getTravelStatsMap()
	{
		return this->travelStatsMap.get();
	}

	bool isQueuing;
	double distanceToEndOfSegment;
	double movingVelocity;

	//for mid-term, to compute link travel times
	travelStats currTravelStats;
	sim_mob::Shared< std::map<double, travelStats> > travelStatsMap; //<linkExitTime, travelStats>
//	double linkEntryTime; //in seconds - time agent change to the current link
//	double roleEntryTime; //in seconds - time agent changed to the current role

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
	bool call_frame_init;

    //Unknown until runtime
    std::map<std::string, std::string> configProperties;

	PendingEvent* currEvent;
	PendingEvent* nextEvent;

	bool nextPathPlanned; //determines if the detailed path for the current subtrip is already planned

	bool onActivity; //Determines if the person is conducting any activity
	long lastUpdatedFrame; //Frame number in which the previous update of this agent took place
	boost::mutex lastUpdatedFrame_mutex;

	//add by xuyan
protected:
	int dynamic_seed;

	//Random number generator
	//TODO: For now (for thread safety) I am giving each Agent control over its own random
	//      number stream. We can probably raise this to the Worker level if we require it.
	boost::mt19937 gen;
	const sim_mob::Link* currLink;
	const sim_mob::Lane* currLane;
	const sim_mob::RoadSegment* currSegment;

	sim_mob::ProfileBuilder* profile;

public:
	//xuyan: old code, might not used any more
	int getOwnRandomNumber();


	bool isCallFrameInit() const {
		return call_frame_init;
	}

	void setCallFrameInit(bool callFrameInit) {
		call_frame_init = callFrameInit;
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
};

} //End namespace sim_mob








