//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DriverCommFacets.hpp"

#include "entities/commsim/broker/Broker.hpp"
#include "entities/Person.hpp"

using namespace sim_mob;
using std::vector;
using std::set;
using std::string;
using std::endl;

using namespace sim_mob;


sim_mob::DriverCommBehavior::DriverCommBehavior(sim_mob::Person* parentAgent):
	DriverBehavior(parentAgent)
{
}

sim_mob::DriverCommBehavior::~DriverCommBehavior()
{
}


sim_mob::DriverCommMovement::DriverCommMovement(sim_mob::Person* parentAgent) :
	DriverMovement(parentAgent)
{
}

sim_mob::DriverCommMovement::~DriverCommMovement()
{
}

void sim_mob::DriverCommMovement::frame_init()
{
	DriverMovement::frame_init();

	//Register this Agent with the Broker singleton.
	Broker::GetSingleBroker()->registerEntity(parent);
}


