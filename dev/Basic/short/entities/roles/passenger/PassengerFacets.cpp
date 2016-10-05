//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PassengerFacets.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/BusStopAgent.hpp"
#include "entities/Person.hpp"
#include "entities/Person_ST.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "logging/Log.hpp"
#include "message/MessageBus.hpp"
#include "message/ST_Message.hpp"

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

PassengerMovement::PassengerMovement() :
MovementFacet(), parentPassenger(nullptr)
{
}

PassengerMovement::~PassengerMovement()
{
}

void PassengerMovement::frame_init() 
{
	//Tele-port the passenger to the destination MRT stop with a delay equal to the travel time
	//Travel is pre-set based on the historic transit times 
	if(parentPassenger->roleType == Role<Person_ST>::RL_TRAINPASSENGER)
	{
		Person_ST *parent = parentPassenger->getParent();
		uint32_t travelTime = parent->currSubTrip->endTime.getValue();
				
		parentPassenger->setStartPoint(parent->currSubTrip->origin);
		parentPassenger->setEndPoint(parent->currSubTrip->destination);
		parentPassenger->setTravelTime(travelTime);
		
		unsigned int tick = ConfigManager::GetInstance().FullConfig().baseGranMS();
		messaging::MessageBus::PostMessage(parent, MSG_WAKEUP_MRT_PAX, messaging::MessageBus::MessagePtr(new PersonMessage(parent)), false,
				travelTime / tick);
	}
}

void PassengerMovement::frame_tick() 
{
	//If the person has to alight the vehicle, set it to be removed
	if(parentPassenger->canAlightVehicle())
	{
		parentPassenger->getParent()->setToBeRemoved();
	}
}

std::string PassengerMovement::frame_tick_output()
{
	return string();
}
