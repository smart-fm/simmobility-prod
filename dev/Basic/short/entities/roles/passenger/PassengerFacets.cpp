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
using namespace messaging;

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
MovementFacet(), parentPassenger(nullptr), totalTimeToComplete(0)
{
}

PassengerMovement::~PassengerMovement()
{
}

void PassengerMovement::frame_init() 
{
	totalTimeToComplete = 0;
	Person_ST *parent = parentPassenger->getParent();
	parentPassenger->setStartPoint(parent->currSubTrip->origin);
	parentPassenger->setEndPoint(parent->currSubTrip->destination);
	
	//Tele-port the passenger (other than bus passenger) to the destination with a delay equal to the travel time
	//Travel time is pre-set based on the historic transit times
	if(parentPassenger->roleType != Role<Person_ST>::RL_PASSENGER)
	{
		totalTimeToComplete = parent->currSubTrip->endTime.getValue();
		parentPassenger->setTravelTime(totalTimeToComplete);
		
		unsigned int tick = ConfigManager::GetInstance().FullConfig().baseGranMS();
		MessageBus::PostMessage(parent, MSG_WAKEUP_MRT_PAX, MessageBus::MessagePtr(new PersonMessage(parent)), false,
				totalTimeToComplete / tick);
	}
}

void PassengerMovement::frame_tick() 
{
	totalTimeToComplete += ConfigManager::GetInstance().FullConfig().baseGranMS();
	
	//If the person has to alight the vehicle, set it to be removed
	if(parentPassenger->canAlightVehicle())
	{
		parentPassenger->setTravelTime(totalTimeToComplete);
		parentPassenger->getParent()->setToBeRemoved();
	}
}

std::string PassengerMovement::frame_tick_output()
{
	return string();
}
