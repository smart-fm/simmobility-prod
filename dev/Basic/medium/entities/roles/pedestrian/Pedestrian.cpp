//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Pedestrian.hpp"
#include "PedestrianFacets.hpp"
#include "entities/Person.hpp"
#include "config/MT_Config.hpp"

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
	PedestrianBehavior* behavior = new PedestrianBehavior(parent);
	PedestrianMovement* movement = nullptr;
	const vector<double> paramsWalkSpeed = MT_Config::GetInstance().getParamsWalkSpeed();
	if(paramsWalkSpeed.size()>0){
		movement = new PedestrianMovement(parent, paramsWalkSpeed[0]);
	}
	else {
		movement = new PedestrianMovement(parent);
	}
	Pedestrian* pedestrian = new Pedestrian(parent, parent->getMutexStrategy(),
			behavior, movement);
	behavior->setParentPedestrian(pedestrian);
	movement->setParentPedestrian(pedestrian);
	return pedestrian;
}

std::vector<BufferedBase*> sim_mob::medium::Pedestrian::getSubscriptionParams() {
	return vector<BufferedBase*>();
}

void sim_mob::medium::Pedestrian::make_frame_tick_params(timeslice now) {}

}
}

