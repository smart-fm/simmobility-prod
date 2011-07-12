#pragma once

#include <vector>
#include <stdlib.h>

#include <boost/thread.hpp>

#include "../constants.h"
#include "Entity.hpp"
#include "../buffering/Buffered.hpp"
#include "../buffering/BufferedDataManager.hpp"
#include "../simple_classes.h"


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

	///Update the agent's shortest path. (Currently does nothing; might not even belong here)
	//void updateShortestPath();

	//Removal methods
	bool isToBeRemoved();
	void setToBeRemoved(bool value);

public:
//	sim_mob::Buffered<unsigned int> xPos;  ///<The agent's position, X
//	sim_mob::Buffered<unsigned int> yPos;  ///<The agent's position, Y
	sim_mob::Buffered<double> xPos;  ///<The agent's position, X
	sim_mob::Buffered<double> yPos;  ///<The agent's position, Y

	//Agents can access all other agents (although they usually do not access by ID)
	static std::vector<Agent*> all_agents;

	//TEMP; we can't link to the config file directly or we get a circular dependency.
	Point topLeft;
	Point lowerRight;
	Point topLeftCrossing;
	Point lowerRightCrossing;

private:
	//unsigned int currMode;
	bool toRemoved;

public:
	//TEMP
	static boost::mutex global_mutex;


//TODO: Move these into the proper location (inheritance, for most of them)
/*public:
	static void pathChoice(Agent& a);
	static void updateDriverBehavior(Agent& a);
	static void updatePedestrianBehavior(Agent& a);
	static void updatePassengerBehavior(Agent& a);*/
};

}

