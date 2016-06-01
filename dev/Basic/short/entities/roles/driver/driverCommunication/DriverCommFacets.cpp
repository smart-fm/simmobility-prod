//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DriverCommFacets.hpp"

#include "entities/commsim/broker/Broker.hpp"
#include "entities/Person_ST.hpp"

using namespace sim_mob;
using std::vector;
using std::set;
using std::string;
using std::endl;

using namespace sim_mob;

DriverCommMovement::DriverCommMovement() :
DriverMovement()
{
}

DriverCommMovement::~DriverCommMovement()
{
}

void DriverCommMovement::frame_init()
{
	DriverMovement::frame_init();

	//Register this Agent with the Broker singleton.
	Broker::GetSingleBroker()->registerEntity(this->getParentDriver()->getParent());
}
