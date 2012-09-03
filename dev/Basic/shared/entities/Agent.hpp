/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <queue>
#include <vector>
#include <functional>
#include <stdlib.h>

#include "GenConfig.h"

#include <boost/thread.hpp>
#include <boost/random.hpp>

#include "util/LangHelpers.hpp"
#include "buffering/Shared.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "geospatial/Point2D.hpp"
#include "conf/simpleconf.hpp"
#include "entities/profile/ProfileBuilder.hpp"

#include "entities/Entity.hpp"
#include "PendingEntity.hpp"
#include "PendingEvent.hpp"

#include "geospatial/Lane.hpp"
#include "geospatial/Link.hpp"
namespace sim_mob
{

class Agent;
class WorkGroup;

#ifndef SIMMOB_DISABLE_MPI
class BoundaryProcessor;
class PackageUtils;
class UnPackageUtils;
#endif

//Comparison for our priority queue
struct cmp_agent_start : public std::less<Person*> {
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
 *
 * Agents maintain an x and a y position. They may have different behavioral models.
 */
class Agent : public sim_mob::Entity {
public:
	///Construct an Agent with an immutable ID.
	///Note that, if -1, the Agent's ID will be assigned automatically. This is the preferred
	///  way of dealing with agent ids. In the case of explicit ID assignment (anything >=0),
	///  the Agent class will take NO measures to ensure that no Agents share IDs.
	explicit Agent(const MutexStrategy& mtxStrat, int id=-1);
	virtual ~Agent();

	///Load an agent.
	virtual void load(const std::map<std::string, std::string>& configProps) = 0;

	virtual Entity::UpdateStatus update(frame_t frameNumber) = 0;  ///<Update agent behavior

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
	//virtual void subscribe(sim_mob::BufferedDataManager* mgr, bool isNew);
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

	//Removal methods
	bool isToBeRemoved();
	void setToBeRemoved();
	void clearToBeRemoved(); ///<Temporary function.

	/* Keeping these methods in Agent class enable to determine the location of the agent without having to
	 * determine the type of the agent and its Role. If it is irrelevant for some sub class of agent to have these methods,
	 * (Signal for example) the sub class can just choose to ignore these. ~ Harish*/
	virtual const sim_mob::Link* getCurrLink() const;
	virtual	void setCurrLink(const sim_mob::Link* link);

	//Getter an setter for only Lane is kept here. Road segment of the agent can be determined from lane.
	virtual const sim_mob::Lane* getCurrLane() const;
	virtual	void setCurrLane(const sim_mob::Lane* lane);

private:
	//For future reference.
	const sim_mob::MutexStrategy mutexStrat;

public:
	//The agent's start/end nodes.
	Node* originNode;
	Node* destNode;

//	sim_mob::Buffered<double> xPos;  ///<The agent's position, X
//	sim_mob::Buffered<double> yPos;  ///<The agent's position, Y

	sim_mob::Shared<int> xPos;  ///<The agent's position, X
	sim_mob::Shared<int> yPos;  ///<The agent's position, Y

	sim_mob::Shared<double> fwdVel;  ///<The agent's velocity, X
	sim_mob::Shared<double> latVel;  ///<The agent's velocity, Y

	sim_mob::Shared<double> xAcc;  ///<The agent's acceleration, X
	sim_mob::Shared<double> yAcc;  ///<The agent's acceleration, Y
	//sim_mob::Buffered<int> currentLink;
	//sim_mob::Buffered<int> currentCrossing;


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


private:
	//unsigned int currMode;
	bool toRemoved;
	static unsigned int next_agent_id;

    //Unknown until runtime
    std::map<std::string, std::string> configProperties;

	PendingEvent* currEvent;
	PendingEvent* nextEvent;

	bool nextPathPlanned; //determines if the detailed path for the current subtrip is already planned

	bool onActivity; //Determines if the person is conducting any activity

	//add by xuyan
protected:
	int dynamic_seed;

	//Random number generator
	//TODO: For now (for thread safety) I am giving each Agent control over its own random
	//      number stream. We can probably raise this to the Worker level if we require it.
	boost::mt19937 gen;
	const sim_mob::Link* currLink;
	const sim_mob::Lane* currLane;

#ifdef SIMMOB_AGENT_UPDATE_PROFILE
	sim_mob::ProfileBuilder profile;
#endif

public:
	//xuyan: old code, might not used any more
	int getOwnRandomNumber();

	friend class BoundaryProcessor;

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

}

