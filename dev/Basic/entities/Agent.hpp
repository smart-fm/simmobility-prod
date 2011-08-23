/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <stdlib.h>

#include <boost/thread.hpp>

#include "../constants.h"
#include "Entity.hpp"
#include "../buffering/Buffered.hpp"
#include "../buffering/BufferedDataManager.hpp"
#include "../geospatial/Point2D.hpp"

#include "../conf/simpleconf.hpp"


namespace sim_mob
{


/**
 * Basic Agent class. Agents maintain an x and a y position. They may have different
 * behavioral models.
 */
class Agent : public sim_mob::Entity {
public:
	Agent(unsigned int id=0);

	virtual void update(frame_t frameNumber) = 0;  ///<Update agent behvaior

	///Subscribe this agent to a data manager.
	//virtual void subscribe(sim_mob::BufferedDataManager* mgr, bool isNew);
	virtual void buildSubscriptionList();

	//Removal methods
	bool isToBeRemoved();
	void setToBeRemoved(bool value);

public:
	//The agent's start/end nodes.
	Node* originNode;
	Node* destNode;
	unsigned int startTime;

	sim_mob::Buffered<double> xPos;  ///<The agent's position, X
	sim_mob::Buffered<double> yPos;  ///<The agent's position, Y

	sim_mob::Buffered<double> xVel;  ///<The agent's velocity, X
	sim_mob::Buffered<double> yVel;  ///<The agent's velocity, Y

	sim_mob::Buffered<double> xAcc;  ///<The agent's acceleration, X
	sim_mob::Buffered<double> yAcc;  ///<The agent's acceleration, Y


	//Agents can access all other agents (although they usually do not access by ID)
	static std::vector<Agent*> all_agents;

private:
	//unsigned int currMode;
	bool toRemoved;

};

}

