/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file Pedestrian2.hpp
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

#include "conf/settings/DisableMPI.h"

#include "entities/roles/Role.hpp"
#include "geospatial/Point2D.hpp"
#include "conf/simpleconf.hpp"
#include "geospatial/Crossing.hpp"
#include "entities/UpdateParams.hpp"
#include "geospatial/RoadSegment.hpp"

#include "PedestrianPathMover.hpp"

using std::vector;

namespace sim_mob
{

class PackageUtils;
class UnPackageUtils;

#ifndef SIMMOB_DISABLE_MPI
class PartitionManager;
#endif

//Helper struct
struct PedestrianUpdateParams2 : public sim_mob::UpdateParams {
	explicit PedestrianUpdateParams2(boost::mt19937& gen) : UpdateParams(gen), skipThisFrame(false) {}
	virtual ~PedestrianUpdateParams2() {}

	virtual void reset(timeslice now)
	{
		sim_mob::UpdateParams::reset(now);

		skipThisFrame = false;
	}

	///Used to skip the first frame; kind of hackish.
	bool skipThisFrame;

#ifndef SIMMOB_DISABLE_MPI
	static void pack(PackageUtils& package, const PedestrianUpdateParams2* params);

	static void unpack(UnPackageUtils& unpackage, PedestrianUpdateParams2* params);
#endif
};
/**
 * A Person in the Pedestrian role is navigating sidewalks and zebra crossings.
 */
class Pedestrian2 : public sim_mob::Role {
public:
	Pedestrian2(Agent* parent, std::string roleName = "pedestrian");
	virtual ~Pedestrian2();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_med(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);
	virtual UpdateParams& make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	bool isOnCrossing() const;
	bool isAtBusStop() const { return isAtBusstop; }

private:
	//Movement-related variables
	double speed;
	double xVel;
	double yVel;

	const Signal* trafficSignal;
	const Crossing* currCrossing;
	int sigColor; //0-red, 1-yellow, 2-green

	//For collisions
	double xCollisionVector;
	double yCollisionVector;
	static double collisionForce;
	static double agentRadius;

	//The following methods are to be moved to agent's sub-systems in future
	void setSubPath();
	void updatePedestrianSignal();
	void checkForCollisions();
	bool checkGapAcceptance();

	//Attempting to replace stage-one movement (TO the intersection) with the GeneralPathMover. ~Seth
	PedestrianPathMover pedMovement;

	//Are we using the multi-path movement model? Set automatically if we move on a path of size >2
	bool isUsingGenPathMover;
	bool isAtBusstop; // indicate whether pedestrian is at stop or not

	//Temporary variable which will be flushed each time tick. We save it
	// here to avoid constantly allocating and clearing memory each time tick.
	PedestrianUpdateParams2 params;

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
