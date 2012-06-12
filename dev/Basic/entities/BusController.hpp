/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * BusController.hpp
 *
 *  Created on: 2012-6-11
 *      Author: Yao Jin
 */

#pragma once

#include <vector>

#include "entities/Agent.hpp"
#include "roles/driver/Driver.hpp"
#include "buffering/Shared.hpp"
#include "entities/UpdateParams.hpp"

namespace sim_mob
{

class BusController : public sim_mob::Agent
{
public:
	explicit BusController(const MutexStrategy& mtxStrat, int id=-1);
	virtual ~BusController();

	virtual Entity::UpdateStatus update(frame_t frameNumber);

	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

	unsigned int getId() const { return id; }

private:
	frame_t frameNumberCheck;// check some frame number to do control
};



}
