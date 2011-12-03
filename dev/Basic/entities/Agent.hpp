/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <queue>
#include <vector>
#include <functional>
#include <stdlib.h>

#include "constants.h"

#include <boost/thread.hpp>

#include "util/LangHelpers.hpp"
#include "buffering/Buffered.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "geospatial/Point2D.hpp"
#include "conf/simpleconf.hpp"

#include "Entity.hpp"




namespace sim_mob
{

class Agent;
class WorkGroup;


//Comparison for our priority queue
struct cmp_agent_start : public std::less<Entity*> {
  bool operator() (const Entity* x, const Entity* y) const;
};


/**
 * Basic Agent class. Agents maintain an x and a y position. They may have different
 * behavioral models.
 */
class Agent : public sim_mob::Entity {
public:
	Agent(int id=-1);
	virtual ~Agent();

	virtual bool update(frame_t frameNumber) = 0;  ///<Update agent behvaior

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

	sim_mob::Buffered<int> xPos;  ///<The agent's position, X
	sim_mob::Buffered<int> yPos;  ///<The agent's position, Y

	//sim_mob::Buffered<double> xVel;  ///<The agent's velocity, X
	//sim_mob::Buffered<double> yVel;  ///<The agent's velocity, Y
	sim_mob::Buffered<double> fwdVel;  //The agent's forward velocity
	sim_mob::Buffered<double> latVel;  //The agent's forward velocity

	//sim_mob::Buffered<double> xAcc;  ///<The agent's acceleration, X
	//sim_mob::Buffered<double> yAcc;  ///<The agent's acceleration, Y
	//sim_mob::Buffered<int> currentLink;
	//sim_mob::Buffered<int> currentCrossing;


	///Agents can access all other agents (although they usually do not access by ID)
	static std::vector<Entity*> all_agents;

#ifndef DISABLE_DYNAMIC_DISPATCH
	//C++ static constructors...
	struct StartTimePriorityQueue {
		std::priority_queue<Entity*> impl;

		StartTimePriorityQueue() : impl(cmp_agent_start()) {}
	};

	static StartTimePriorityQueue pending_agents; //Agents waiting to be added to the simulation, prioritized by start time.

	///When adding/deleting Agents asynchronously, a lock is required.
	static boost::mutex all_agents_lock;
#endif

	///Temporary variable; holds a pointer to the current Agent work group. Will be moved
	///  into the Dispatch Manager as soon as it's created.
	//static sim_mob::WorkGroup* TMP_AgentWorkGroup;

	///Retrieve a monotonically-increasing unique ID value.
	///\param preferredID Will be returned if it is greater than the current maximum-assigned ID.
	///\note
	///Passing in a negative number will always auto-assign an ID, and is recommended.
	static unsigned int GetAndIncrementID(int preferredID);


private:
	//unsigned int currMode;
	bool toRemoved;
	static unsigned int next_agent_id;

};

}

