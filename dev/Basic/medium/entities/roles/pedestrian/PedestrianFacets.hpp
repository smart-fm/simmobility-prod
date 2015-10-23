/*
 * PedestrainFacets.h
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#pragma once

#include "entities/roles/RoleFacets.hpp"
#include "entities/Person.hpp"
#include "geospatial/RoadSegment.hpp"

namespace sim_mob {

class MRT_Stop;

namespace medium {

class Pedestrian;

class PedestrianBehavior: public BehaviorFacet {
public:
	explicit PedestrianBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~PedestrianBehavior();

	//Virtual overrides
	virtual void frame_init() {
	}
	virtual void frame_tick() {
	}
	virtual void frame_tick_output() {
	}

	/**
	 * set parent reference to pedestrian.
	 * @param parentPedestrian is pointer to parent pedestrian
	 */
	void setParentPedestrian(sim_mob::medium::Pedestrian* parentPedestrian);

protected:
	sim_mob::medium::Pedestrian* parentPedestrian;

};

class PedestrianMovement: public MovementFacet {
public:
	explicit PedestrianMovement(sim_mob::Person* parentAgent, double speed);
	virtual ~PedestrianMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();
	virtual sim_mob::Conflux* getStartingConflux() const;

	/**
	 * set parent reference to pedestrian.
	 * @param parentPedestrian is pointer to parent pedestrian
	 */
	void setParentPedestrian(sim_mob::medium::Pedestrian* parentPedestrian);

	TravelMetric & startTravelTimeMetric();
	TravelMetric & finalizeTravelTimeMetric();
protected:

	/**
	 * initialize the path at the beginning
	 * @param path include aPathSetParams list of road segments
	 * */
	const sim_mob::RoadSegment* getDestSegment();

	/**parent pedestrian*/
	sim_mob::medium::Pedestrian* parentPedestrian;

	/**record the current remaining time to the destination*/
	double remainingTimeToComplete;

	/**pedestrian's walking speed*/
	const double walkSpeed;

	/**destination segment*/
	const sim_mob::RoadSegment* destinationSegment;

	/** total time to complete in seconds*/
	double totalTimeToCompleteSec;

	/**seconds in tick*/
	double secondsInTick;
};

}
}
