//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Pedestrian.hpp"
#include "PedestrianFacets.hpp"
#include "entities/Person.hpp"
#include "config/MT_Config.hpp"
#include "message/MT_Message.hpp"
#include "entities/PT_Statistics.hpp"
#include "entities/roles/activityRole/ActivityFacets.hpp"
#include "entities/Person_MT.hpp"

using std::vector;
using namespace sim_mob;

namespace sim_mob
{

namespace medium
{

sim_mob::medium::Pedestrian::Pedestrian(Person_MT *parent,
										sim_mob::medium::PedestrianBehavior* behavior,
										sim_mob::medium::PedestrianMovement* movement,
										std::string roleName, Role::type roleType) :
sim_mob::Role(parent, behavior, movement, parent, roleName, roleType)
{
}

sim_mob::medium::Pedestrian::~Pedestrian()
{
}

Role* sim_mob::medium::Pedestrian::clone(Person_MT *parent) const
{
	double walkSpeed = MT_Config::getInstance().getPedestrianWalkSpeed();
	PedestrianBehavior* behavior = new PedestrianBehavior();
	PedestrianMovement* movement = new PedestrianMovement(walkSpeed);
	Pedestrian* pedestrian = new Pedestrian(parent, behavior, movement);
	behavior->setParentPedestrian(pedestrian);
	movement->setParentPedestrian(pedestrian);
	return pedestrian;
}

std::vector<BufferedBase*> sim_mob::medium::Pedestrian::getSubscriptionParams()
{
	return vector<BufferedBase*>();
}

void sim_mob::medium::Pedestrian::make_frame_tick_params(timeslice now)
{
}

}

void sim_mob::medium::Pedestrian::collectTravelTime()
{
	std::string personId, tripStartPoint, tripEndPoint, subStartPoint,
			subEndPoint, subStartType, subEndType, mode, service, arrivaltime,
			travelTime;

	personId = boost::lexical_cast<std::string>(parent->getId());
	if (parent->getPrevRole() && parent->getPrevRole()->roleType == Role::RL_ACTIVITY)
	{
		ActivityPerformer* activity = dynamic_cast<ActivityPerformer*> (parent->getPrevRole());
		tripStartPoint = boost::lexical_cast<std::string>(activity->getLocation()->getID());
		tripEndPoint = boost::lexical_cast<std::string>(activity->getLocation()->getID());
		subStartPoint = boost::lexical_cast<std::string>(activity->getLocation()->getID());
		subEndPoint = boost::lexical_cast<std::string>(activity->getLocation()->getID());
		subStartType = "NODE";
		subEndType = "NODE";
		mode = parent->currSubTrip->getMode();
		service = parent->currSubTrip->ptLineId;
		travelTime = DailyTime(activity->getTravelTime()).getStrRepr();
		arrivaltime = DailyTime(activity->getArrivalTime()).getStrRepr();
		mode = "ACTIVITY";
		messaging::MessageBus::PostMessage(PT_Statistics::GetInstance(),
										STORE_PERSON_TRAVEL,
										messaging::MessageBus::MessagePtr(new PersonTravelTimeMessage(personId, tripStartPoint,
																									  tripEndPoint, subStartPoint, subEndPoint,
																									  subStartType, subEndPoint, mode, service,
																									  arrivaltime, travelTime)), true);
	}
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
	mode = "WALK";
	messaging::MessageBus::PostMessage(PT_Statistics::GetInstance(),
									STORE_PERSON_TRAVEL,
									messaging::MessageBus::MessagePtr(new PersonTravelTimeMessage(personId, tripStartPoint,
																								  tripEndPoint, subStartPoint, subEndPoint,
																								  subStartType, subEndType, mode, service,
																								  arrivaltime, travelTime)), true);

}
}

