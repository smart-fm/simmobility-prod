/*
 * Basic Agent class
 */

#pragma once

#include "../constants.h"


//Driver modes
enum DRIVER_MODES {
	DRIVER,
	PEDESTRIAN,
	CYCLIST,
	PASSENGER
};


class Agent {
public:
	Agent(unsigned int id=0);

	virtual void update();

private:
	unsigned int id;
	unsigned int currMode;

//Trivial accessors/mutators. Header-implemented
public:
	unsigned int getId() const { return id; }


//TODO: Move these into the proper location (inheritance, for most of them)
public:
	static void pathChoice(Agent& a);
	static void updateDriverBehavior(Agent& a);
	static void updatePedestrianBehavior(Agent& a);
	static void updatePassengerBehavior(Agent& a);
};
