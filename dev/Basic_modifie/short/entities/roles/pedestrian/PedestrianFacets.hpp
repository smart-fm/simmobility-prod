//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>

#include "entities/roles/RoleFacets.hpp"

namespace sim_mob
{

class Pedestrian;

class PedestrianBehaviour : public BehaviorFacet
{
private:
	Pedestrian *parentPedestrian;
	
public:
	PedestrianBehaviour() : BehaviorFacet(), parentPedestrian(NULL)
	{
	}
	
	virtual ~PedestrianBehaviour()
	{
	}

	//Virtual overrides
	virtual void frame_init()
	{
		throw std::runtime_error("Not implemented: PedestrianBehavior::frame_init()");
	}
	
	virtual void frame_tick()
	{
		throw std::runtime_error("Not implemented: PedestrianBehavior::frame_tick()");
	}
	
	virtual std::string frame_tick_output()
	{
		throw std::runtime_error("Not implemented: PedestrianBehavior::frame_tick_output()");
	}
	
	/**
	 * set parent reference to pedestrian.
	 * @param parentPedestrian is pointer to parent pedestrian
	 */
	void setParentPedestrian(Pedestrian *parentPedestrian)
	{
		this->parentPedestrian = parentPedestrian;
	}
};

class PedestrianMovement : public MovementFacet
{
private:
	Pedestrian* parentPedestrian;
	
	/**Distance to be covered by walking*/
	double distanceToBeCovered;
	
public:
	explicit PedestrianMovement();
	virtual ~PedestrianMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();

	// mark startTimeand origin
	virtual TravelMetric& startTravelTimeMetric()
	{
		return travelMetric;
	}

	//	mark the destination and end time and travel time
	virtual TravelMetric& finalizeTravelTimeMetric()
	{
		return travelMetric;
	}

	Pedestrian* getParentPedestrian() const
	{
		return parentPedestrian;
	}

	void setParentPedestrian(Pedestrian *parentPedestrian)
	{
		this->parentPedestrian = parentPedestrian;
	}
};

}

