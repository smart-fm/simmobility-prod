#pragma once

#include "Role.hpp"

#include <time.h>
#include <math.h>

namespace sim_mob
{

/**
 * A Person in the Pedestrian role is navigating sidewalks and zebra crossings.
 */
class Pedestrian : public sim_mob::Role {
public:
	Pedestrian();

	virtual void update();

private:
	//Movement-related variables
	double speed;
	double xVel;
	double yVel;
	Point goal;
	bool isGoalSet;
	unsigned int currPhase; //Current pedestrian signal phase: 0-green, 1-red
	unsigned int phaseCounter; //To be replaced by traffic management system

	//For collisions
	double xCollisionVector;
	double yCollisionVector;
	static double collisionForce;
	static double agentRadius;

	//The following methods are to be moved to agent's sub-systems in future
	bool isGoalReached();
	void setGoal();
	void updateVelocity();
	void updatePosition();
	void updatePedestrianSignal();
	void checkForCollisions();
	bool reachStartOfCrossing();

};



}
