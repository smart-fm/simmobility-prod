//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Biker.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

sim_mob::medium::Biker::Biker(Person_MT* parent, sim_mob::medium::BikerBehavior* behavior, sim_mob::medium::BikerMovement* movement, std::string roleName,
		Role<Person_MT>::Type roleType) :
		sim_mob::medium::Driver(parent, behavior, movement, roleName, roleType)
{
}

sim_mob::medium::Biker::~Biker()
{
}

Role<Person_MT>* sim_mob::medium::Biker::clone(Person_MT* parent) const
{
	BikerBehavior* behavior = new BikerBehavior();
	BikerMovement* movement = new BikerMovement();
	Biker* biker = new Biker(parent, behavior, movement, "Biker_");
	behavior->setParentBiker(biker);
	movement->setParentBiker(biker);
	movement->setParentDriver(biker);
	return biker;
}
