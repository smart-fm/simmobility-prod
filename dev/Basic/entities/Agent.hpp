/*
 * Basic Agent class
 */

#pragma once

#include "../constants.h"
#include "Entity.hpp"


//Driver modes
enum DRIVER_MODES {
	DRIVER,
	PEDESTRIAN,
	CYCLIST,
	PASSENGER
};


class Agent : public Entity {
public:
	Agent(unsigned int id=0);

	virtual void update();

	void updateShortestPath();

private:
	unsigned int currMode;


//TODO: Move these into the proper location (inheritance, for most of them)
public:
	static void pathChoice(Agent& a);
	static void updateDriverBehavior(Agent& a);
	static void updatePedestrianBehavior(Agent& a);
	static void updatePassengerBehavior(Agent& a);
};
