//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "WaitBusActivityFacets.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/BusStopAgent.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "message/MessageBus.hpp"
#include "message/ST_Message.hpp"
#include "WaitBusActivity.hpp"

using namespace sim_mob;

WaitBusActivityMovement::WaitBusActivityMovement() :
MovementFacet(), parentWaitBusActivity(nullptr)
{
}

WaitBusActivityMovement::~WaitBusActivityMovement()
{
}

void WaitBusActivityMovement::setParentWaitBusActivity(WaitBusActivity *parentWaitBusActivity)
{
	this->parentWaitBusActivity = parentWaitBusActivity;
}

void WaitBusActivityMovement::frame_init()
{
	if(parentWaitBusActivity)
	{
		UpdateParams &params = parentWaitBusActivity->getParams();
		Person *person = parentWaitBusActivity->parent;
		person->setStartTime(params.now.ms());
	}
}

void WaitBusActivityMovement::frame_tick()
{
	unsigned int tickMS = ConfigManager::GetInstance().FullConfig().baseGranMS();
	
	switch(parentWaitBusActivity->activityState)
	{
	case WAITBUS_STATE_WAITING:		
		parentWaitBusActivity->increaseWaitingTime(tickMS);
		parentWaitBusActivity->setTravelTime(parentWaitBusActivity->getWaitingTime());
		break;

	case WAITBUS_STATE_DECIDED_BOARD_BUS:		
		//Waiting person has decided to board the bus, send attempting to board message to bus driver
		parentWaitBusActivity->activityState = WAITBUS_STATE_ATTEMPTED_BOARD_BUS;
		messaging::MessageBus::PostMessage(parentWaitBusActivity->busDriver->getParent(), MSG_ATTEMPT_BOARD_BUS,
				messaging::MessageBus::MessagePtr(new PersonMessage(parentWaitBusActivity->getParent())));				
		break;
		
	case WAITBUS_STATE_ATTEMPTED_BOARD_BUS:
		//Do nothing, wait for bus driver's response
		break;
		
	case WAITBUS_STATE_WAIT_COMPLETE:
		//Waiting role complete
		parentWaitBusActivity->parent->setToBeRemoved();
		break;
	}	
}

std::string WaitBusActivityMovement::frame_tick_output()
{
	return std::string();
}

TravelMetric& WaitBusActivityMovement::startTravelTimeMetric()
{
	return travelMetric;
}

TravelMetric& WaitBusActivityMovement::finalizeTravelTimeMetric()
{
	return travelMetric;
}
