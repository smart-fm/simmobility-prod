//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "DriverFacets.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "DriverUpdateParams.hpp"
#include "BusDriver.hpp"

/*
 * BusDriverFacets.hpp
 *
 */

namespace sim_mob {
namespace medium
{
class BusDriver;

class BusDriverBehavior: public DriverBehavior {
public:
	explicit BusDriverBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~BusDriverBehavior();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();


	void setParentBusDriver(sim_mob::medium::BusDriver* parentBusDriver) {
		this->parentBusDriver = parentBusDriver;
	}

protected:
	sim_mob::medium::BusDriver* parentBusDriver;
};

class BusDriverMovement: public DriverMovement {
public:
	explicit BusDriverMovement(sim_mob::Person* parentAgent = nullptr);
	virtual ~BusDriverMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();
	void flowIntoNextLinkIfPossible(DriverUpdateParams& p);

	sim_mob::medium::BusDriver* getParentBusDriver() const {
		return parentBusDriver;
	}

	void setParentBusDriver(sim_mob::medium::BusDriver* parentBusDriver) {
		this->parentBusDriver = parentBusDriver;
	}

protected:
	virtual bool initializePath();

	sim_mob::medium::BusDriver* parentBusDriver;
};

}
}
