//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * \file SimplenergyModel.hpp
 *
 * \author Michael Choi
 */

#pragma once

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include "EnergyModelBase.hpp"
#include "tripEnergySO/tripEnergySO.h"
#include "util/DailyTime.hpp"

#include <array>
namespace sim_mob
{
namespace medium
{
class SegmentStats;

/**
 * SimpleEnergyModel is a sample energy Model class that overrides the necessary virtual functions for EnergyModelBase.
 *
 *
 * \author Michael Choi
 */

class TripEnergyModel: public EnergyModelBase {
public:
	TripEnergyModel();
	virtual ~TripEnergyModel();

	//Virtual overrides
	virtual void computeEnergyWithSpeedHolder(const std::deque<double> speedCollector, struct0_T &vehicleStruct, const double timeStep);
	virtual void computeTrainEnergyWithSpeed(const double trainMovement, struct0_T &vehicleStruct, const double timeStep); 
	virtual struct0_T initVehicleStruct(const std::string drivetrain);

	// jo{ Mar 13, need these for now
	virtual std::pair<double, double> computeEnergyWithVelocityVector( //const VehicleParams& vp, //jo Mar13
			const std::vector<double> velocityVector, double timeStep, std::string vehicleTypeStr);
	virtual void setupParamsVariables();
	//jo
protected:
	double moments[438976];
	double spdBins[38];
//	bool trip_output;
//	bool segment_output;
	bool timestep_output = false;
	bool timestep_title_written = false;
	std::string timestep_file;
};

} // end namespace medium
} // end namespace sim_mob
