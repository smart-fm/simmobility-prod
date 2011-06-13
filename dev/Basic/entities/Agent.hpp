/*
 * Basic Agent class
 */

#pragma once



#include "../constants.h"
#include "Entity.hpp"
#include "../buffering/Buffered.hpp"
#include "../buffering/BufferedDataManager.hpp"


namespace sim_mob
{

//Driver modes
enum DRIVER_MODES {
	DRIVER,
	PEDESTRIAN,
	CYCLIST,
	PASSENGER
};


class Agent : public sim_mob::Entity {
public:
	Agent(unsigned int id=0);

	virtual void update();
	virtual void subscribe(sim_mob::BufferedDataManager* mgr, bool isNew);

	void updateShortestPath();

public:
	sim_mob::Buffered<unsigned int> xPos;
	sim_mob::Buffered<unsigned int> yPos;

private:
	unsigned int currMode;


//TODO: Move these into the proper location (inheritance, for most of them)
public:
	static void pathChoice(Agent& a);
	static void updateDriverBehavior(Agent& a);
	static void updatePedestrianBehavior(Agent& a);
	static void updatePassengerBehavior(Agent& a);
};

}

