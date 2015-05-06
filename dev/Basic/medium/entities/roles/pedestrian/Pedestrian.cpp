//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Pedestrian.hpp"
#include "PedestrianFacets.hpp"
#include "entities/Person.hpp"
#include "config/MT_Config.hpp"
#include "message/MT_Message.hpp"
#include "entities/PT_Statistics.hpp"

using std::vector;
using namespace sim_mob;

namespace sim_mob {

namespace medium {

sim_mob::medium::Pedestrian::Pedestrian(Person* parent, MutexStrategy mtxStrat,
		sim_mob::medium::PedestrianBehavior* behavior,
		sim_mob::medium::PedestrianMovement* movement,
		std::string roleName, Role::type roleType) :
		sim_mob::Role(behavior, movement, parent, roleName, roleType )
{}

sim_mob::medium::Pedestrian::~Pedestrian() {}

Role* sim_mob::medium::Pedestrian::clone(Person* parent) const {
	double walkSpeed = MT_Config::getInstance().getPedestrianWalkSpeed();
	PedestrianBehavior* behavior = new PedestrianBehavior(parent);
	PedestrianMovement* movement = new PedestrianMovement(parent, walkSpeed);
	Pedestrian* pedestrian = new Pedestrian(parent, parent->getMutexStrategy(), behavior, movement);
	behavior->setParentPedestrian(pedestrian);
	movement->setParentPedestrian(pedestrian);
	return pedestrian;
}

std::vector<BufferedBase*> sim_mob::medium::Pedestrian::getSubscriptionParams() {
	return vector<BufferedBase*>();
}

void sim_mob::medium::Pedestrian::make_frame_tick_params(timeslice now) {}

}

void sim_mob::medium::Pedestrian::collectTravelTime()
{
	std::string personId, startPoint, endPoint, mode, service, arrivaltime,
			travelTime;
	personId = boost::lexical_cast<std::string>(parent->GetId());
	startPoint = parent->currSubTrip->fromLocationId;
	endPoint = parent->currSubTrip->toLocationId;
	mode = parent->currSubTrip->getMode();
	service = parent->currSubTrip->ptLineId;
	travelTime = DailyTime(parent->getRole()->getTravelTime()).toString();

	messaging::MessageBus::PostMessage(PT_Statistics::GetInstance(),
			STORE_PERSON_TRAVEL,
			messaging::MessageBus::MessagePtr(
					new PersonTravelTimeMessage(personId, startPoint, endPoint,
							mode, service, arrivaltime, travelTime)));

}
}

