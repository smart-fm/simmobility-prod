//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PassengerFacets.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/Person.hpp"
#include "entities/BusStopAgent.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "logging/Log.hpp"
#include "entities/Person_ST.hpp"

using namespace sim_mob;

PassengerBehavior::PassengerBehavior() :
BehaviorFacet(), parentPassenger(nullptr)
{
}

PassengerBehavior::~PassengerBehavior()
{
}

void PassengerBehavior::frame_init()
{
	throw std::runtime_error("PassengerBehavior::frame_init is not implemented yet");
}

void PassengerBehavior::frame_tick()
{
	throw std::runtime_error("PassengerBehavior::frame_tick is not implemented yet");
}

std::string PassengerBehavior::frame_tick_output()
{
	throw std::runtime_error("PassengerBehavior::frame_tick_output is not implemented yet");
}

sim_mob::PassengerMovement::PassengerMovement() :
MovementFacet(), parentPassenger(nullptr)
{
}

sim_mob::PassengerMovement::~PassengerMovement()
{
}

void sim_mob::PassengerMovement::frame_init() 
{
}

void sim_mob::PassengerMovement::frame_tick() 
{
}

std::string sim_mob::PassengerMovement::frame_tick_output()
{
	return string();
}
