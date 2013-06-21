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
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);

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
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);

	DriverComm* getParentDriverComm() const {
		return parentDriverCommRole;
	}

	void setParentDriverComm(DriverComm* parentDriverCommRole_) {
		this->parentDriverCommRole = parentDriverCommRole_;
	}

protected:
	DriverComm* parentDriverCommRole;

};
}
