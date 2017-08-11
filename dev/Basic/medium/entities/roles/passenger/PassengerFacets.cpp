/*
 * PassengerFacets.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#include "PassengerFacets.hpp"
#include "config/MT_Config.hpp"
#include "geospatial/network/Node.hpp"
#include "Passenger.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/driver/TaxiDriver.hpp"
#include "entities/roles/driver/TaxiDriverFacets.hpp"
#include "entities/roles/driver/DriverFacets.hpp"




using namespace sim_mob;
using namespace medium;

PassengerBehavior::PassengerBehavior() : BehaviorFacet(), parentPassenger(nullptr)
{
}

PassengerBehavior::~PassengerBehavior()
{
}

PassengerMovement::PassengerMovement() : MovementFacet(), parentPassenger(nullptr), totalTimeToCompleteMS(0)
{
}

PassengerMovement::~PassengerMovement()
{
}

void PassengerMovement::setParentPassenger(Passenger* parentPassenger)
{
	this->parentPassenger = parentPassenger;
}

void PassengerBehavior::setParentPassenger(Passenger* parentPassenger)
{
	this->parentPassenger = parentPassenger;
}

void PassengerMovement::frame_init()
{
	totalTimeToCompleteMS = 0;
}

void PassengerMovement::frame_tick()
{
	unsigned int tickMS = ConfigManager::GetInstance().FullConfig().baseGranMS();
	totalTimeToCompleteMS += tickMS;
	parentPassenger->setTravelTime(totalTimeToCompleteMS);
	parentPassenger->getParent()->setRemainingTimeThisTick(0);
}

std::string PassengerMovement::frame_tick_output()
{
	return std::string();
}

TravelMetric & PassengerMovement::startTravelTimeMetric()
{
	travelMetric.startTime = DailyTime(parentPassenger->getArrivalTime());
	travelMetric.origin = parentPassenger->getStartPoint();
	if(parentPassenger->getDriver())
	{
		if (parentPassenger->getDriver()->roleType == Role<Person_MT>::RL_TAXIDRIVER)
		{
			travelMetric.startTime = DailyTime(const_cast<Driver*>(parentPassenger->getDriver())->getParams().now.ms())
										+ DailyTime(ConfigManager::GetInstance().FullConfig().simStartTime());
		}
	}
	travelMetric.started = true;
	return travelMetric;
}

TravelMetric & PassengerMovement::finalizeTravelTimeMetric()
{
	travelMetric.destination = parentPassenger->getEndPoint();
	travelMetric.endTime = DailyTime(parentPassenger->getArrivalTime() + parentPassenger->totalTravelTimeMS);
	travelMetric.travelTime = TravelMetric::getTimeDiffHours(travelMetric.endTime , travelMetric.startTime); // = totalTimeToCompleteMS in hours
	if(parentPassenger->getDriver())
	{
		if (parentPassenger->getDriver()->roleType == Role<Person_MT>::RL_TAXIDRIVER)
		{
			travelMetric.distance = parentPassenger->getFinalPointDriverDistance() - parentPassenger->getStartPointDriverDistance();
			travelMetric.endTime =DailyTime(const_cast<Driver*>(parentPassenger->getDriver())->getParams().now.ms())
			                      + DailyTime(ConfigManager::GetInstance().FullConfig().simStartTime());
			travelMetric.travelTime = TravelMetric::getTimeDiffHours(travelMetric.endTime , travelMetric.startTime);
			parentPassenger->setDriver(NULL);
		}
	}
		travelMetric.finalized = true;
	return travelMetric;
}

Conflux* PassengerMovement::getDestinationConflux() const
{
	if (parentPassenger->roleType == Role<Person_MT>::RL_CARPASSENGER
			|| parentPassenger->roleType == Role<Person_MT>::RL_PRIVATEBUSPASSENGER
			|| parentPassenger->roleType == Role<Person_MT>::RL_TRAINPASSENGER)
	{
		return MT_Config::getInstance().getConfluxForNode(parentPassenger->parent->currSubTrip->destination.node);
	}
	return nullptr;
}
