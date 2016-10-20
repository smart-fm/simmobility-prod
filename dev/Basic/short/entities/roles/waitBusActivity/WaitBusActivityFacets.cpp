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
MovementFacet(), parentWaitBusActivity(nullptr), isMessageSent(false)
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
	
	if(!parentWaitBusActivity->hasBoardedBus)
	{
		if (!parentWaitBusActivity->decidedToBoardBus)
		{
			//Waiting person can't board bus, continue to wait
			parentWaitBusActivity->increaseWaitingTime(tickMS);
			parentWaitBusActivity->setTravelTime(parentWaitBusActivity->getWaitingTime());
		}
		else if(!isMessageSent)
		{
			//Waiting person has decided to board the bus, send attempting to board message to bus driver
			messaging::MessageBus::PostMessage(parentWaitBusActivity->busDriver->getParent(), MSG_ATTEMPT_BOARD_BUS,
					messaging::MessageBus::MessagePtr(new PersonMessage(parentWaitBusActivity->getParent())));
			
			isMessageSent = true;
		}
	}
	else
	{
		//Waiting role complete
		parentWaitBusActivity->parent->setToBeRemoved();
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
