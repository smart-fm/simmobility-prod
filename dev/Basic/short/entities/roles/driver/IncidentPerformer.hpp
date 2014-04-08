//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * IncidentPerformer.hpp
 *
 *  Created on: Mar 25, 2014
 *      Author: zhang
 */

#include "entities/IncidentStatus.hpp"
#include "DriverUpdateParams.hpp"

#pragma once

namespace sim_mob {

class Driver;
class IncidentPerformer {
public:
	IncidentPerformer();
	virtual ~IncidentPerformer();

	/**
	 * get incident status.
	 * @return incident status object.
	 */
	sim_mob::IncidentStatus& getIncidentStatus();

	/**
	 * Check current incident status.
	 * @param paren is pointer to the class Driver
	 * @param p is reference to the DriverUpdateParam which recording parameters for updating driver
	 * @param now is current simulation time
	 * @return void .
	 */
	void checkIncidentStatus(Driver* parent, DriverUpdateParams& p, timeslice now);

	/**
	 * response processing when incident happen.
	 * @param paren is pointer to the class Driver
	 * @param p is reference to the DriverUpdateParam which recording parameters for updating driver
	 * @param now is current simulation time
	 * @return void .
	 */
	void responseIncidentStatus(Driver* parent, DriverUpdateParams& p, timeslice now);


private:
	//incident response plan
	sim_mob::IncidentStatus incidentStatus;


	/**
	 * check whether ahead vehicles exists.
	 * @param paren is pointer to the class Driver
	 * @param p is reference to the DriverUpdateParam which recording parameters for updating driver
	 * @return void .
	 */
	void checkAheadVehicles(Driver* parent, DriverUpdateParams& p);

	/**
	 * calculate new speed by current acceleration.
	 * @param curSpeed is current velocity
	 * @param acc is current acceleration
	 * @param elapsedSeconds is elapsed time during current frame tick
	 * @return void .
	 */
	inline float calculateSpeedbyAcceleration(float curSpeed, float acc, float elapsedSeconds);

};

} /* namespace sim_mob */

