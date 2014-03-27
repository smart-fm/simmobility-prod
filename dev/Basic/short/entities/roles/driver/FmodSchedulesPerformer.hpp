//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * FMODSchedulesPerformer.hpp
 *
 *  Created on: Mar 27, 2014
 *      Author: zhang
 */
#include "DriverUpdateParams.hpp"

#pragma once

namespace sim_mob {

class Driver;
class FmodSchedulesPerformer {
public:
	FmodSchedulesPerformer();
	virtual ~FmodSchedulesPerformer();

	bool performFmodSchedule(Driver* parentDriver, DriverUpdateParams& p);

private:
	bool processTripsInSchedule(Driver* parentDriver, FMODSchedule* schedule, FMODSchedule::STOP& stopSchedule, DriverUpdateParams& p);

};


} /* namespace sim_mob */

