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

#include "GenConfig.h"

#include "entities/roles/Role.hpp"
#include "geospatial/Point2D.hpp"
#include "conf/simpleconf.hpp"
#include "entities/Signal.hpp"
#include "geospatial/Crossing.hpp"
#include "entities/roles/driver/GeneralPathMover.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

namespace sim_mob
{

#ifndef SIMMOB_DISABLE_MPI
class PartitionManager;
#endif

//Stages
enum PedestrianStage {
	ApproachingIntersection = 0,
	NavigatingIntersection = 1,
	LeavingIntersection =2
};

//Prefix increment operator for Stage
inline void operator++(PedestrianStage& rhs) {
  switch (rhs) {
	  case ApproachingIntersection:
		  rhs = NavigatingIntersection;
		  break;
	  case NavigatingIntersection:
		  rhs = LeavingIntersection;
		  break;
	  default:
		  throw std::runtime_error("Cannot increment LeavingIntersection");
  }
}


/**
 * A Person in the Pedestrian role is navigating sidewalks and zebra crossings.
 */
class Pedestrian : public sim_mob::Role {
public:
	Pedestrian(Agent* parent);
	virtual ~Pedestrian();

	virtual void update(frame_t frameNumber);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();
	void output(frame_t frameNumber);
	bool isOnCrossing() const;

private:
	//Movement-related variables
	double speed;
	double xVel;
	double yVel;
	Point2D goal;
	Point2D goalInLane;
	PedestrianStage currentStage;

//	Signal sig;
	const Signal* trafficSignal;
	const Crossing* currCrossing;
	int sigColor; //0-red, 1-yellow, 2-green
//	unsigned int phaseCounter; //To be replaced by traffic management system
	int curCrossingID;
	bool startToCross;
	double cStartX, cStartY, cEndX, cEndY;
	bool firstTimeUpdate;
	Point2D interPoint;

	//For collisions
	double xCollisionVector;
	double yCollisionVector;
	static double collisionForce;
	static double agentRadius;

	//The following methods are to be moved to agent's sub-systems in future
	bool isGoalReached();
	bool isDestReached();
	void setGoal(PedestrianStage currStage,const RoadSegment* prevSegment);
	void updateVelocity(int);
	void updatePosition();
	void updatePedestrianSignal();
	void checkForCollisions();
//	bool reachStartOfCrossing();
	bool checkGapAcceptance();
	void setCrossingParas(const RoadSegment* prevSegment); //Temp helper function
	bool isFirstTimeUpdate(); //Temp helper function
	void setSidewalkParas(Node* start, Node* end, bool isStartMulti);
	void absToRel(double, double, double &, double &);
	void relToAbs(double, double, double &, double &);

	//Attempting to replace stage-one movement (TO the intersection) with the GeneralPathMover. ~Seth
	GeneralPathMover fwdMovement;

	//Could be folded into the code if we switched goal checking and movement.
	const RoadSegment* prevSeg;

	//Are we using the multi-path movement model? Set automatically if we move on a path of size >2
	bool isUsingGenPathMover;

	//Serialization
#ifndef SIMMOB_DISABLE_MPI
public:
	friend class sim_mob::PartitionManager;

public:
	virtual void package(PackageUtils& packageUtil);
	virtual void unpackage(UnPackageUtils& unpackageUtil);

	virtual void packageProxy(PackageUtils& packageUtil);
	virtual void unpackageProxy(UnPackageUtils& unpackageUtil);
#endif
};


}
