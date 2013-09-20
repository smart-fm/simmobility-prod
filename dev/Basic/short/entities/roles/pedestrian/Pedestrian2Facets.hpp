//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * Pedestrian2Facets.hpp
 *
 *  Created on: May 14th, 2013
 *      Author: Yao Jin
 */

#pragma once
#include "conf/settings/DisableMPI.h"
#include "entities/roles/RoleFacets.hpp"
#include "Pedestrian2.hpp"
#include "PedestrianPathMover.hpp"

namespace sim_mob {

class Pedestrian2;
class Signal;

class Pedestrian2Behavior: public sim_mob::BehaviorFacet {
public:
	explicit Pedestrian2Behavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~Pedestrian2Behavior();

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);

	Pedestrian2* getParentPedestrian2() const {
		return parentPedestrian2;
	}

	void setParentPedestrian2(Pedestrian2* parentPedestrian2) {
		this->parentPedestrian2 = parentPedestrian2;
	}

private:
	Pedestrian2* parentPedestrian2;

};

class Pedestrian2Movement: public sim_mob::MovementFacet {
public:
	explicit Pedestrian2Movement(sim_mob::Person* parentAgent = nullptr);
	virtual ~Pedestrian2Movement();

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void flowIntoNextLinkIfPossible(UpdateParams& p);

	bool isOnCrossing() const;
	Pedestrian2* getParentPedestrian2() const {
		return parentPedestrian2;
	}
	void setParentPedestrian2(Pedestrian2* parentPedestrian2) {
		this->parentPedestrian2 = parentPedestrian2;
	}

private:
	void setSubPath();
	void updatePedestrianSignal();
	void checkForCollisions();
	bool checkGapAcceptance();

private:
	Pedestrian2* parentPedestrian2;
	//Movement-related variables
	double speed;
	double xVel;
	double yVel;

	const Signal* trafficSignal;// later move this into Pedestrian2 if required by Behavior Facet
	const Crossing* currCrossing;
	int sigColor; //0-red, 1-yellow, 2-green

	//For collisions
	double xCollisionVector;
	double yCollisionVector;
	static double collisionForce;
	static double agentRadius;

	//Attempting to replace stage-one movement (TO the intersection) with the GeneralPathMover. ~Seth
	PedestrianPathMover pedMovement;

	//Are we using the multi-path movement model? Set automatically if we move on a path of size >2
	bool isUsingGenPathMover;
};

}
