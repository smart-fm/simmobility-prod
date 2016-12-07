/*
 * WaitTaxiActivity.cpp
 *
 *  Created on: Nov 28, 2016
 *      Author: zhang huai peng
 */
#include <vector>
#include "WaitTaxiActivity.hpp"
#include "entities/Person_MT.hpp"
#include "geospatial/network/TaxiStand.hpp"
#include "entities/PT_Statistics.hpp"
#include "message/MT_Message.hpp"
#include "WaitTaxiActivityFacets.hpp"

namespace sim_mob
{
namespace medium
{
WaitTaxiActivity::WaitTaxiActivity(Person_MT* parent,
		WaitTaxiActivityBehavior* behavior, WaitTaxiActivityMovement* movement,
		const TaxiStand* stand,
		std::string roleName, Role<Person_MT>::Type roleType) :
		sim_mob::Role<Person_MT>::Role(parent, behavior, movement, roleName,
				roleType), stand(stand), waitingTime(0)
{
}

WaitTaxiActivity::~WaitTaxiActivity() {

}

sim_mob::Role<Person_MT>* WaitTaxiActivity::clone(Person_MT *parent) const
{
	SubTrip& subTrip = *(parent->currSubTrip);
	if(subTrip.origin.type!=WayPoint::TAXI_STAND){
		throw std::runtime_error("Waiting taxi activity do not have stand!");
	}
	WaitTaxiActivityBehavior* behavior = new WaitTaxiActivityBehavior();
	WaitTaxiActivityMovement* movement = new WaitTaxiActivityMovement();
	WaitTaxiActivity* waitTaxiActivity = new WaitTaxiActivity(parent, behavior,	movement, subTrip.origin.taxiStand);
	behavior->setWaitTaxiActivity(waitTaxiActivity);
	movement->setWaitTaxiActivity(waitTaxiActivity);
	return waitTaxiActivity;
}

void WaitTaxiActivity::make_frame_tick_params(timeslice now)
{
	getParams().reset(now);
}

void WaitTaxiActivity::HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
}

std::vector<BufferedBase*> WaitTaxiActivity::getSubscriptionParams()
{
	return std::vector<BufferedBase*>();
}

void WaitTaxiActivity::collectTravelTime()
{
	PersonTravelTime personTravelTime;
	std::string personId, tripStartPoint, tripEndPoint, subStartPoint, subEndPoint, subStartType, subEndType, mode, service, arrivaltime, travelTime;
	personTravelTime.personId = parent->getDatabaseId();
	personTravelTime.tripStartPoint = (*(parent->currTripChainItem))->startLocationId;
	personTravelTime.tripEndPoint = (*(parent->currTripChainItem))->endLocationId;
	personTravelTime.subStartPoint = parent->currSubTrip->startLocationId;
	personTravelTime.subEndPoint = parent->currSubTrip->endLocationId;
	personTravelTime.subStartType = parent->currSubTrip->startLocationType;
	personTravelTime.subEndType = parent->currSubTrip->endLocationType;
	personTravelTime.mode = "WAITING_TAXIS";
	personTravelTime.travelTime = ((double) parent->getRole()->getTravelTime())/1000.0; //convert to seconds
	personTravelTime.arrivalTime = DailyTime(parent->getRole()->getArrivalTime()).getStrRepr();
	messaging::MessageBus::PostMessage(PT_Statistics::getInstance(),
					STORE_PERSON_TRAVEL_TIME, messaging::MessageBus::MessagePtr(new PersonTravelTimeMessage(personTravelTime)), true);
}

void WaitTaxiActivity::increaseWaitingTime(unsigned int timeMs)
{
	waitingTime += timeMs;
}
unsigned int WaitTaxiActivity::getWaitingTime() const
{
	return waitingTime;
}
const TaxiStand* WaitTaxiActivity::getTaxiStand() const
{
	return stand;
}
}
}

