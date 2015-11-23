//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <vector>

#include "BusDriver.hpp" 
#include "conf/settings/DisableMPI.h"
#include "entities/roles/passenger/Passenger.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "entities/vehicle/Bus.hpp"

namespace sim_mob
{

class BusDriver;

class BusDriverBehavior : public DriverBehavior
{
protected:
	BusDriver* parentBusDriver;

public:
	explicit BusDriverBehavior() : DriverBehavior(), parentBusDriver(nullptr)
	{
	}
	
	virtual ~BusDriverBehavior()
	{		
	}

	virtual void frame_init()
	{
		throw std::runtime_error("Not implemented: BusDriverBehavior::frame_init()");
	}
	
	virtual void frame_tick()
	{
		throw std::runtime_error("Not implemented: BusDriverBehavior::frame_tick()");
	}
	
	virtual std::string frame_tick_output()
	{
		throw std::runtime_error("Not implemented: BusDriverBehavior::frame_tick_output()");
	}

	BusDriver* getParentBusDriver() const
	{
		return parentBusDriver;
	}

	void setParentBusDriver(BusDriver* parentBusDriver)
	{
		if (!parentBusDriver)
		{
			throw std::runtime_error("parentBusDriver cannot be NULL");
		}
		this->parentBusDriver = parentBusDriver;
	}
} ;

class BusDriverMovement : public DriverMovement
{
private:
	/**The bus stops*/
	std::vector<const BusStop *> busStops;
	/**
	 * Initialises the bus path using the bus route information
	 * 
	 * @param createVehicle indicates whether a new vehicle is to be created
	 * 
	 * @return the new vehicle, if requested to build one
	 */
	Vehicle* initialiseBusPath(bool createVehicle);
	
	/**
	 * Builds a path of way-points consisting of road segments and turning groups using the given road-segments
	 * 
	 * @param routeId the bus route id
	 * @param pathOfSegments the road segments in the path
	 */
	void buildPath(const std::string &routeId, const std::vector<const RoadSegment *> &pathOfSegments);
	
protected:
	/**The bus driver*/
	BusDriver *parentBusDriver;
	
public:
	explicit BusDriverMovement();
	virtual ~BusDriverMovement();

	/**
	 * This method is called for the first tick of the agent's role as a bus driver. The method initialises the path,
	 * allocates a vehicle to the driver and does other initialisation tasks
     */
	virtual void frame_init();

	/**
	 * This method is called every frame tick and is responsible for the bus driver's movement and all decisions leading to
	 * the movement during the tick
     */
	virtual void frame_tick();

	/**
	 * This method outputs the parameters that changed at the end of the tick
     */
	virtual std::string frame_tick_output();

	void setParentBusDriver(BusDriver *parentBusDriver)
	{
		if (!parentBusDriver)
		{
			throw std::runtime_error("parentBusDriver cannot be NULL");
		}
		
		this->parentBusDriver = parentBusDriver;
	}
};
}
