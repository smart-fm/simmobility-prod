/*
 * PedestrainFacets.h
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#pragma once
#include "entities/conflux/Conflux.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "entities/Person.hpp"
#include "geospatial/network/Node.hpp"

namespace sim_mob
{

class TrainStop;

namespace medium
{

class Pedestrian;

class PedestrianBehavior : public BehaviorFacet
{
public:
	explicit PedestrianBehavior();
	virtual ~PedestrianBehavior();

	//Virtual overrides

	virtual void frame_init()
	{
	}

	virtual void frame_tick()
	{
	}

	virtual std::string frame_tick_output()
	{
		return std::string();
	}

	/**
	 * set parent reference to pedestrian.
	 * @param parentPedestrian is pointer to parent pedestrian
	 */
	void setParentPedestrian(medium::Pedestrian* parentPedestrian);

protected:
	medium::Pedestrian* parentPedestrian;

};

class PedestrianMovement : public MovementFacet
{
public:
	explicit PedestrianMovement(double speed);
	virtual ~PedestrianMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();
	virtual Conflux* getStartingConflux() const;

	/**
	 * set parent reference to pedestrian.
	 * @param parentPedestrian is pointer to parent pedestrian
	 */
	void setParentPedestrian(medium::Pedestrian* parentPedestrian);

	TravelMetric & startTravelTimeMetric();
	TravelMetric & finalizeTravelTimeMetric();
protected:

	/**
	 * initialize the path at the beginning
	 * @param path include aPathSetParams list of road segments
	 * */
	const Node* getDestNode();

	/**parent pedestrian*/
	medium::Pedestrian* parentPedestrian;

	/**record the current remaining time to the destination*/
	double remainingTimeToComplete;

	/**pedestrian's walking speed*/
	const double walkSpeed;

	/**destination segment*/
	const Node* destinationNode;

	/** total time to complete in seconds*/
	double totalTimeToCompleteSec;

	/**seconds in tick*/
	double secondsInTick;
};

}
}
