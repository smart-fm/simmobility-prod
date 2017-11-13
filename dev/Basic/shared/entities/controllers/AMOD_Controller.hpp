//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "OnCallController.hpp"

namespace sim_mob
{

class AMOD_Controller : public OnCallController
{
protected:
	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	virtual void computeSchedules();

public:
	AMOD_Controller(const MutexStrategy &mtx, unsigned int computationPeriod, unsigned int id,
	                TT_EstimateType tt_estType);
};

}