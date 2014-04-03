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

void sim_mob::DriverCommBehavior::frame_init()
{
	DriverBehavior::frame_init();
}

void sim_mob::DriverCommBehavior::frame_tick()
{
	DriverBehavior::frame_tick();
}

void sim_mob::DriverCommBehavior::frame_tick_output()
{
	DriverBehavior::frame_tick_output();
}


sim_mob::DriverCommMovement::DriverCommMovement(sim_mob::Person* parentAgent):
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

void sim_mob::DriverCommMovement::frame_tick()
{
	DriverMovement::frame_tick();
}

void sim_mob::DriverCommMovement::frame_tick_output()
{
	DriverMovement::frame_tick_output();
}


/*DriverComm* sim_mob::DriverCommMovement::getParentDriverComm() const
{
	return parentDriverCommRole;
}

void sim_mob::DriverCommMovement::setParentDriverComm(DriverComm* parentDriverCommRole_)
{
	this->parentDriverCommRole = parentDriverCommRole_;
}*/

