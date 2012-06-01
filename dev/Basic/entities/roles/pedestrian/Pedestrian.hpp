/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file Pedestrian.hpp
 *
 * \author Luo Linbo
 * \author Seth N. Hetu
 * \author Li Zhemin
 * \author LIM Fung Chai
 * \author Xu Yan
 */

#pragma once

#include <time.h>
#include <math.h>

#include <boost/random.hpp>

#include "GenConfig.h"

#include "entities/roles/Role.hpp"
#include "geospatial/Point2D.hpp"
#include "conf/simpleconf.hpp"
#ifdef NEW_SIGNAL
#include "entities/signal/Signal.hpp"
#else
#include "entities/Signal.hpp"
#endif
#include "geospatial/Crossing.hpp"
#include "entities/roles/driver/GeneralPathMover.hpp"
#include "entities/UpdateParams.hpp"

namespace sim_mob
{

class PackageUtils;
class UnPackageUtils;

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

//Helper struct
struct PedestrianUpdateParams : public sim_mob::UpdateParams {
	explicit PedestrianUpdateParams(boost::mt19937& gen) : UpdateParams(gen) {}

	virtual void reset(frame_t frameNumber, unsigned int currTimeMS)
	{
		sim_mob::UpdateParams::reset(frameNumber, currTimeMS);

		skipThisFrame = false;
	}

	///Used to skip the first frame; kind of hackish.
	bool skipThisFrame;

#ifndef SIMMOB_DISABLE_MPI
	static void pack(PackageUtils& package, const PedestrianUpdateParams* params);

	static void unpack(UnPackageUtils& unpackage, PedestrianUpdateParams* params);
#endif
};



/**
 * A Person in the Pedestrian role is navigating sidewalks and zebra crossings.
 */
class Pedestrian : public sim_mob::Role {
public:
	Pedestrian(Agent* parent, boost::mt19937& gen);
	virtual ~Pedestrian();

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(frame_t frameNumber);
	virtual UpdateParams& make_frame_tick_params(frame_t frameNumber, unsigned int currTimeMS);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

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
	void setCrossingParas(const RoadSegment* prevSegment, boost::mt19937& gen); //Temp helper function
	void setSidewalkParas(Node* start, Node* end, bool isStartMulti);
	void absToRel(double, double, double &, double &);
	void relToAbs(double, double, double &, double &);

	//Attempting to replace stage-one movement (TO the intersection) with the GeneralPathMover. ~Seth
	GeneralPathMover fwdMovement;

	//Could be folded into the code if we switched goal checking and movement.
	const RoadSegment* prevSeg;

	//Are we using the multi-path movement model? Set automatically if we move on a path of size >2
	bool isUsingGenPathMover;

	//Temporary variable which will be flushed each time tick. We save it
	// here to avoid constantly allocating and clearing memory each time tick.
	PedestrianUpdateParams params;

	//Serialization-related friends
	friend class PackageUtils;
	friend class UnPackageUtils;

#ifndef SIMMOB_DISABLE_MPI
public:
	friend class PartitionManager;

	virtual void pack(PackageUtils& packageUtil);
	virtual void unpack(UnPackageUtils& unpackageUtil);

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif
};


}
