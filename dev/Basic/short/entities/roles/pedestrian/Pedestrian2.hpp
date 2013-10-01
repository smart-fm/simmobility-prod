//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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
#include "geospatial/Crossing.hpp"
#include "entities/UpdateParams.hpp"
#include "geospatial/RoadSegment.hpp"

#include "entities/roles/pedestrian/PedestrianPathMover.hpp"
#include "Pedestrian2Facets.hpp"

using std::vector;

namespace sim_mob
{
class Pedestrian2Behavior;
class Pedestrian2Movement;
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
	Pedestrian2(Agent* parent, sim_mob::Pedestrian2Behavior* behavior = nullptr, sim_mob::Pedestrian2Movement* movement = nullptr, Role::type roleType_ = RL_PEDESTRIAN, std::string roleName = "pedestrian");
	virtual ~Pedestrian2();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Virtual overrides
	virtual UpdateParams& make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

private:
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
