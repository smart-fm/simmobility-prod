//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Passenger.hpp"
#include "PassengerFacets.hpp"
#include "entities/Person.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "message/MT_Message.hpp"
#include "message/MT_Message.hpp"
#include "entities/PT_Statistics.hpp"
#include "entities/Person_MT.hpp"
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
Role<Person_MT>(parent, behavior, movement, roleName, roleType),
driver(nullptr), alightBus(false), startNode(nullptr), endNode(nullptr)
{
}

Role<Person_MT>* sim_mob::medium::Passenger::clone(Person_MT *parent) const
{
	PassengerBehavior* behavior = new PassengerBehavior();
	PassengerMovement* movement = new PassengerMovement();
	Role<Person_MT>::Type roleType = Role<Person_MT>::RL_PASSENGER;
	if (parent->currSubTrip->mode == "MRT")
	{
		roleType = Role<Person_MT>::RL_TRAINPASSENGER;
	}
	else if (parent->currSubTrip->mode == "Sharing")
	{
		roleType = Role<Person_MT>::RL_CARPASSENGER;
	}
	Passenger* passenger = new Passenger(parent, behavior, movement, "Passenger_", roleType);
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
	if (parent->destNode.type_ == WayPoint::BUS_STOP
			&& parent->destNode.busStop_ == nextStop)
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
	std::string personId, tripStartPoint, tripEndPoint, subStartPoint,
			subEndPoint, subStartType, subEndType, mode, service, arrivaltime,
			travelTime;

	personId = boost::lexical_cast<std::string>(parent->getId());
	tripStartPoint = (*(parent->currTripChainItem))->startLocationId;
	tripEndPoint = (*(parent->currTripChainItem))->endLocationId;
	subStartPoint = parent->currSubTrip->startLocationId;
	subEndPoint = parent->currSubTrip->endLocationId;
	subStartType = parent->currSubTrip->startLocationType;
	subEndType = parent->currSubTrip->endLocationType;
	mode = parent->currSubTrip->getMode();
	service = parent->currSubTrip->ptLineId;
	travelTime = DailyTime(parent->getRole()->getTravelTime()).getStrRepr();
	arrivaltime = DailyTime(parent->getRole()->getArrivalTime()).getStrRepr();
	if (roleType == Role<Person_MT>::RL_TRAINPASSENGER)
	{
		mode = "MRT_TRAVEL";
	}
	else if (roleType == Role<Person_MT>::RL_CARPASSENGER)
	{
		mode = "CARSHARING_TRAVEL";
	}
	else
	{
		mode = "BUS_TRAVEL";
	}

	messaging::MessageBus::PostMessage(PT_Statistics::GetInstance(),
									STORE_PERSON_TRAVEL,
									messaging::MessageBus::MessagePtr(
																	  new PersonTravelTimeMessage(personId, tripStartPoint,
																								  tripEndPoint, subStartPoint, subEndPoint,
																								  subStartType, subEndType, mode, service,
																								  arrivaltime, travelTime)), true);
}

}
}
