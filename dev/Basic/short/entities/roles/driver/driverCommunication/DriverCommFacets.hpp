//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * DriverFacets.hpp
 *
 *  Created on: May 15th, 2013
 *      Author: Yao Jin
 */

#pragma once
#include "entities/roles/driver/DriverFacets.hpp"
#include "entities/roles/driver/driverCommunication/DriverComm.hpp"

namespace sim_mob {

class DriverCommBehavior: public sim_mob::DriverBehavior {
public:
	explicit DriverCommBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~DriverCommBehavior();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	DriverComm* getParentDriverComm() const {
		return parentDriverCommRole;
	}

	void setParentDriverComm(DriverComm* parentDriverCommRole_) {
		this->parentDriverCommRole = parentDriverCommRole_;
	}

protected:
	DriverComm* parentDriverCommRole;

};

class DriverCommMovement: public sim_mob::DriverMovement {

public:
	explicit DriverCommMovement(sim_mob::Person* parentAgent = nullptr);
	virtual ~DriverCommMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	DriverComm* getParentDriverComm() const;

	void setParentDriverComm(DriverComm* parentDriverCommRole_);

protected:
	DriverComm* parentDriverCommRole;

};
}
