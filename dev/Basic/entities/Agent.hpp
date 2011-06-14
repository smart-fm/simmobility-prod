#pragma once

#include <boost/thread.hpp>

#include "../constants.h"
#include "Entity.hpp"
#include "../buffering/Buffered.hpp"
#include "../buffering/BufferedDataManager.hpp"


namespace sim_mob
{

/**
 * Possible agent behaviors.
 *
 * \todo
 * Represent agent behavior using inheritance, instead.
 */
enum AGENT_MODES {
	DRIVER,
	PEDESTRIAN,
	CYCLIST,
	PASSENGER
};



/**
 * Basic Agent class. Agents maintain an x and a y position. They may have different
 * behavioral models.
 */
class Agent : public sim_mob::Entity {
public:
	Agent(unsigned int id=0);

	virtual void update();  ///<Update agent behvaior

	///Subscribe this agent to a data manager.
	virtual void subscribe(sim_mob::BufferedDataManager* mgr, bool isNew);

	///Update the agent's shortest path. (Currently does nothing; might not even belong here)
	void updateShortestPath();

public:
	sim_mob::Buffered<unsigned int> xPos;  ///<The agent's position, X
	sim_mob::Buffered<unsigned int> yPos;  ///<The agent's position, Y

private:
	unsigned int currMode;

	//TEMP
	static boost::mutex global_mutex;


//TODO: Move these into the proper location (inheritance, for most of them)
public:
	static void pathChoice(Agent& a);
	static void updateDriverBehavior(Agent& a);
	static void updatePedestrianBehavior(Agent& a);
	static void updatePassengerBehavior(Agent& a);
};

}

