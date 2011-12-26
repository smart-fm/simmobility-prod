/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "GenConfig.h"

#include <queue>
#include <vector>
#include <functional>
#include <cstdlib>

#include <boost/thread.hpp>
#include <boost/random.hpp>

#include "entities/Entity.hpp"

#include "util/LangHelpers.hpp"
#include "buffering/Shared.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "geospatial/Point2D.hpp"
#include "conf/simpleconf.hpp"


#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

namespace sim_mob
{

class Agent;
class WorkGroup;
class UpdateParams;

#ifndef SIMMOB_DISABLE_MPI
class BoundaryProcessor;
//class PackageUtils;
//class UnPackageUtils;
#endif

//Comparison for our priority queue
struct cmp_agent_start : public std::less<Entity*> {
  bool operator() (const Entity* x, const Entity* y) const;
};

//C++ static constructors...
class StartTimePriorityQueue : public std::priority_queue<Entity*, std::vector<Entity*>, cmp_agent_start> {
};


/**
 * Basic Agent class. Agents maintain an x and a y position. They may have different
 * behavioral models.
 */
class Agent : public sim_mob::Entity {
public:
	Agent(const MutexStrategy& mtxStrat, int id=-1);
	virtual ~Agent();

	///Update this Agent. Performs life-cycle management, then calls frame_tick.
	///Note: Sub-classes of Agent should override frame_tick, NOT update. If you want
	///      to override update, you should probably be extending Entity.
	virtual bool update(frame_t frameNumber) final;


	///Called the first time an Agent's update() method is successfully called.
	/// This will be the tick of its startTime, rounded down(?).
	virtual void frame_init(UpdateParams& p) = 0;

	///Perform each frame's update tick for this Agent.
	virtual bool frame_tick(UpdateParams& p) = 0;

	///Generate output for this frame's tick for this Agent.
	virtual void frame_tick_output(const UpdateParams& p) = 0;

	///Create the UpdateParams (or, more likely, sub-class) which will hold all
	///  the temporary information for this time tick.
	virtual UpdateParams& make_frame_tick_params(frame_t frameNumber, unsigned int currTimeMS) = 0;

	///Subscribe this agent to a data manager.
	//virtual void subscribe(sim_mob::BufferedDataManager* mgr, bool isNew);
	virtual void buildSubscriptionList();

	//Removal methods
	bool isToBeRemoved();
	void setToBeRemoved();

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

	//Agents waiting to be added to the simulation, prioritized by start time.
	static StartTimePriorityQueue pending_agents;

	///Retrieve a monotonically-increasing unique ID value.
	///\param preferredID Will be returned if it is greater than the current maximum-assigned ID.
	///\note
	///Passing in a negative number will always auto-assign an ID, and is recommended.
	static unsigned int GetAndIncrementID(int preferredID);

	///Note: Calling this function from another Agent is extremely dangerous if you
	/// don't know what you're doing.
	boost::mt19937& getGenerator() {
		return gen;
	}


private:
	bool firstFrameTick;  ///Determines if frame_init() has been done.
	bool toRemoved;
	static unsigned int next_agent_id;

	//add by xuyan
protected:
	int dynamic_seed;

	//Random number generator
	//TODO: For now (for thread safety) I am giving each Agent control over its own random
	//      number stream. We can probably raise this to the Worker level if we require it.
	boost::mt19937 gen;

public:
	int getOwnRandomNumber();

#ifndef SIMMOB_DISABLE_MPI
public:
	friend class BoundaryProcessor;
	/**
	 * Used for crossing agents
	 */
	virtual void package(PackageUtils& packageUtil);
	virtual void unpackage(UnPackageUtils& unpackageUtil);

	/**
	 * used for feedback and feed forward agents
	 */
	virtual void packageProxy(PackageUtils& packageUtil);
	virtual void unpackageProxy(UnPackageUtils& unpackageUtil);
#endif
};

}

