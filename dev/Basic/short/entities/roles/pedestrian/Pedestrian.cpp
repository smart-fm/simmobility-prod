//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <vector>

#include "Pedestrian.hpp"
#include "entities/Person_ST.hpp"

using namespace std;
using namespace sim_mob;

Pedestrian::Pedestrian(Person_ST *parent, PedestrianBehaviour *behaviour, PedestrianMovement *movement, Role<Person_ST>::Type roleType_, std::string roleName) :
Role<Person_ST>::Role(parent, behaviour, movement, roleName, roleType_)
{
}

Pedestrian::~Pedestrian()
{
}

Role<Person_ST>* Pedestrian::clone(Person_ST *parent) const
{
	PedestrianBehaviour *behaviour = new PedestrianBehaviour();
	PedestrianMovement *movement = new PedestrianMovement();
	Pedestrian *pedestrian = new Pedestrian(parent, behaviour, movement);
	behaviour->setParentPedestrian(pedestrian);
	movement->setParentPedestrian(pedestrian);
	return pedestrian;
}

vector<BufferedBase *> Pedestrian::getSubscriptionParams()
{
	return vector<BufferedBase *>();
}

void Pedestrian::make_frame_tick_params(timeslice now)
{
}
