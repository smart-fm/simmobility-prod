//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include "entities/roles/driver/DriverFacets.hpp"
#include "entities/roles/driver/driverCommunication/DriverComm.hpp"

namespace sim_mob {

class DriverCommBehavior: public sim_mob::DriverBehavior {
public:
	explicit DriverCommBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~DriverCommBehavior();
};

class DriverCommMovement: public sim_mob::DriverMovement {
public:
	explicit DriverCommMovement(sim_mob::Person* parentAgent = nullptr);
	virtual ~DriverCommMovement();

	//Virtual overrides
	virtual void frame_init();
};
}
