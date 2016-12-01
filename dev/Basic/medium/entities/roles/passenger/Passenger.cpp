//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Passenger.hpp"
#include "entities/PT_Statistics.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "message/MT_Message.hpp"
#include "message/MT_Message.hpp"
#include "PassengerFacets.hpp"

using std::vector;
using namespace sim_mob;

namespace sim_mob
{

namespace medium
{

sim_mob::medium::Passenger::Passenger(Person_MT *parent, 
									  sim_mob::medium::PassengerBehavior* behavior,
									  sim_mob::medium::PassengerMovement* movement,
									  std::string roleName, Role<Person_MT>::Type roleType) :
				Role<Person_MT>(parent, behavior, movement, roleName, roleType), driver(nullptr), alightBus(false)
{
}

Role<Person_MT>* sim_mob::medium::Passenger::clone(Person_MT *parent) const
{
	PassengerBehavior* behavior = new PassengerBehavior();
	PassengerMovement* movement = new PassengerMovement();
	Role<Person_MT>::Type personRoleType = Role<Person_MT>::RL_UNKNOWN;
	if (parent->currSubTrip->getMode() == "MRT")
	{
		personRoleType = Role<Person_MT>::RL_TRAINPASSENGER;
	}
	else if (parent->currSubTrip->getMode() == "Sharing")
	{
		personRoleType = Role<Person_MT>::RL_CARPASSENGER;
	}
	else if (parent->currSubTrip->getMode() == "PrivateBus")
	{
		personRoleType = Role<Person_MT>::RL_PRIVATEBUSPASSENGER;
	}
	else if (parent->currSubTrip->getMode() == "BusTravel")
	{
		personRoleType = Role<Person_MT>::RL_PASSENGER;
	}
	else if (parent->currSubTrip->getMode() == "TaxiTravel")
	{
		personRoleType = Role<Person_MT>::RL_TAXIPASSENGER;
	}
	else
	{
		throw std::runtime_error("Unknown mode for passenger role");
	}
	Passenger* passenger = new Passenger(parent, behavior, movement, "Passenger_", personRoleType);
	behavior->setParentPassenger(passenger);
	movement->setParentPassenger(passenger);
	return passenger;
}

std::vector<BufferedBase*> sim_mob::medium::Passenger::getSubscriptionParams()
{
	return vector<BufferedBase*>();
}

void sim_mob::medium::Passenger::makeAlightingDecision(const sim_mob::BusStop* nextStop)
{
	if (parent->destNode.type == WayPoint::BUS_STOP
			&& parent->destNode.busStop == nextStop)
	{
		setAlightBus(true);
		setDriver(nullptr);
	}
}

void sim_mob::medium::Passenger::HandleParentMessage(messaging::Message::MessageType type,
													 const messaging::Message& message)
{
	switch (type)
	{
	case ALIGHT_BUS:
	{
		const BusStopMessage& msg = MSG_CAST(BusStopMessage, message);
		makeAlightingDecision(msg.nextStop);
		break;
	}
	default:
	{
		break;
	}
	}
}

void sim_mob::medium::Passenger::collectTravelTime()
{
	PersonTravelTime personTravelTime;
	personTravelTime.personId = parent->getDatabaseId();
	personTravelTime.tripStartPoint = (*(parent->currTripChainItem))->startLocationId;
	personTravelTime.tripEndPoint = (*(parent->currTripChainItem))->endLocationId;
	personTravelTime.subStartPoint = parent->currSubTrip->startLocationId;
	personTravelTime.subEndPoint = parent->currSubTrip->endLocationId;
	personTravelTime.subStartType = parent->currSubTrip->startLocationType;
	personTravelTime.subEndType = parent->currSubTrip->endLocationType;
	personTravelTime.mode = parent->currSubTrip->getMode();
	personTravelTime.service = parent->currSubTrip->ptLineId;
	personTravelTime.travelTime = ((double)parent->getRole()->getTravelTime()) / 1000.0; //convert to seconds
	personTravelTime.arrivalTime = DailyTime(parent->getRole()->getArrivalTime()).getStrRepr();
	if (roleType == Role<Person_MT>::RL_TRAINPASSENGER)
	{
		personTravelTime.mode = "MRT_TRAVEL";
	}
	else if (roleType == Role<Person_MT>::RL_CARPASSENGER)
	{
		personTravelTime.mode = "CAR_SHARING_TRAVEL";
	}
	else if (roleType == Role<Person_MT>::RL_PRIVATEBUSPASSENGER)
	{
		personTravelTime.mode = "PRIVATE_BUS_TRAVEL";
	}
	else
	{
		personTravelTime.mode = "BUS_TRAVEL";
	}

	messaging::MessageBus::PostMessage(PT_Statistics::getInstance(),
			STORE_PERSON_TRAVEL_TIME, messaging::MessageBus::MessagePtr(new PersonTravelTimeMessage(personTravelTime)), true);
}

}
}
