/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Pedestrian.hpp
 *
 *  Created on: 2011-6-20
 *      Author: Linbo
 */

#pragma once

#include <time.h>
#include <math.h>

#include "entities/roles/Role.hpp"
#include "geospatial/Point2D.hpp"
#include "conf/simpleconf.hpp"
#include "entities/Signal.hpp"
#include "geospatial/Crossing.hpp"

namespace sim_mob
{

/**
 * A Person in the Pedestrian role is navigating sidewalks and zebra crossings.
 */
class Pedestrian : public sim_mob::Role {
public:
	Pedestrian(Agent* parent);

	virtual void update(frame_t frameNumber);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();
	bool isOnCrossing() const;

private:
	//Movement-related variables
	double speed;
	double xVel;
	double yVel;
	Point2D goal;
	int currentStage;

//	Signal sig;
	const Signal* trafficSignal;
	const Crossing* currCrossing;
	int sigColor; //0-red, 1-yellow, 2-green
//	unsigned int phaseCounter; //To be replaced by traffic management system
	int curCrossingID;
	bool startToCross;
	double cStartX, cStartY, cEndX, cEndY;
	bool startPosSet;
//	Point2D destPos;

	//For collisions
	double xCollisionVector;
	double yCollisionVector;
	static double collisionForce;
	static double agentRadius;

	//The following methods are to be moved to agent's sub-systems in future
	bool isGoalReached();
	bool isDestReached();
	void setGoal(int);
	void updateVelocity(int);
	void updatePosition();
	void updatePedestrianSignal();
	void checkForCollisions();
//	bool reachStartOfCrossing();
	bool checkGapAcceptance();
	void setCrossingParas(); //Temp helper function
	bool isStartPosSet(); //Temp helper function
	void absToRel(double, double, double &, double &);
	void relToAbs(double, double, double &, double &);

};



}
