/*
 * PedestrainFacets.h
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#pragma once

#include "entities/roles/RoleFacets.hpp"
#include "entities/Person.hpp"

namespace sim_mob {

class Conflux;
namespace medium
{

class Pedestrian;

class PedestrianBehavior: public BehaviorFacet {
public:
	explicit PedestrianBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~PedestrianBehavior();

	//Virtual overrides
	virtual void frame_init() {}
	virtual void frame_tick() {}
	virtual void frame_tick_output() {}

	void setParentPedestrian(sim_mob::medium::Pedestrian* parentPedestrian);

protected:
	sim_mob::medium::Pedestrian* parentPedestrian;

};

class PedestrianMovement: public MovementFacet {
public:
	explicit PedestrianMovement(sim_mob::Person* parentAgent = nullptr);
	virtual ~PedestrianMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	void setParentPedestrian(sim_mob::medium::Pedestrian* parentPedestrian);

protected:

	/**
	 * initialize the path at the beginning
	 * @param path include a list of road segments
	 * */
	void initializePath(std::vector<const RoadSegment*>& path);

protected:
	sim_mob::medium::Pedestrian* parentPedestrian;
	//record the current remaining time to the destination
	int remainingTimeToComplete;
	//pedestrian's walking speed
	const float walkSpeed;
	std::vector< std::pair<Conflux*, double> > trajectory;
};

}
}
