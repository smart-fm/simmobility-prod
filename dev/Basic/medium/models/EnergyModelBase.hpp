//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * \file EnergyModelBase.hpp
 *
 * \author Michael Choi
 */

#pragma once

#include <string>
#include "../../shared/behavioral/params/PersonParams.hpp"
#include "entities/conflux/SegmentStats.hpp"
#include "entities/Person_MT.hpp"
#include "util/OneTimeFlag.hpp"
#include "util/Profiler.hpp"
#include "entities/roles/driver/MesoPathMover.hpp"
#include "entities/roles/driver/DriverFacets.hpp"
//#include "entities/roles/driver/TrainDriver.hpp" //jo -Mar27; StopPOintEntity declaration...
//#include "entities/roles/driver/TrainDriverFacets.hpp" //jo- Mar27; TrainMovement is NOT child of DriverMovement

#include "../../shared/entities/misc/TrainTrip.hpp"
#include <array>

namespace sim_mob {
namespace medium {

/**
 * This class implements the container functions to compute energy consumed by a vehicle with different metrics.
 *
 * \author Michael Choi
 */
class EnergyModelBase {
public:
	EnergyModelBase();
	virtual ~EnergyModelBase();

	/**
	 * Virtual function called by computeEnergyBySubTrip. An instantiation is required for any EnergyModel sub-class
	 *
	 * @return true if data was saved to TrajectoryInfo_tthe file; false otherwise.
	 */

	virtual void setupParamsVariables() = 0;

	/**
	 * Computes energy consumed during a subtrip. This function is called in DriverFacets.cpp
	 * 	in three locations (DriverMovement::{frame_tick, advance, moveToNextSegment}), after
	 * 	a subtrip has been "finished".
	 * 	This method gets LT-Supply-related information about vehicle through the MT_Person->PersonParams->VehicleParams.
	 * 	the vehicle
	 * <p>
	 * This method doesn't return anything, but writes the energy computed into energyModelOutputFile
	 * <p>
	 * <b>NOTE: This is currently the way that energy is computed in the simulation, as opposed to computeEnergyByFrameTick</b>
	 *
	 * @param p* pointer to a Person_MT instance that is conducting the trip
	 * @param driverVelocities vector of velocities computed at every timeStep of the trip
	 * @param timeStep the interval in which velocities are queried. This is defaulted to the 'baseGranSeconds' value set in
	 * 	the config file (and accessed through ConfigManager::GetInstance().FullConfig().baseGranSecond())
	 * @see Person_MT
	 */
	//void computeEnergyBySubTrip(const Person_MT* p, const std::vector<double>& driverVelocities, double timeStep);
	void onTripCompletion(const DriverMovement* dM, const Person_MT* p, const std::vector<double>& velocityVector, double timeStep);
	void onBusTripCompletion(const DriverMovement* dM, const Person_MT* busDriver, const std::vector<double>& velocityVector, double timeStep); // jo - Mar25
	void onTrainTripCompletion(const trajectory_info_t& trajectory,//const TrainMovement* tM, //jo Mar27 - taken out for now until fixable TODO!
			const Person_MT* p, const std::vector<double>& velocityVector, double timeStep, int occupancy); // jo - Mar27
	void onOnCallTripCompletion(const DriverMovement* dM, const Person_MT* onCallDriver, const std::vector<double>& velocityVector, double timeStep);
	void onOnHailTripCompletion(const DriverMovement* dM, const Person_MT* taxiDriver, const std::vector<double>& velocityVector, double timeStep); //jo Apr11

	/**
	 * Virtual function called by computeEnergyBySubTrip. An instantiation is required for any EnergyModel sub-class
	 *
	 * @param vp& VehicleParams object (contains properties e.g. drivetrain necessary for computing energy)
	 * @param velocityVector vector of velocities computed at every timeStep of the trip
	 * @param timeStep the interval in which velocities are queried.
	 * @return true if data was saved to the file; false otherwise.
	 */
//	virtual std::pair<double, double> computeEnergyWithVelocityVector( // const VehicleParams& vp, //jo Mar13
//			const std::vector<double> velocityVector, double timeStep, std::string vehicleTypeStr) = 0;

	//aa{
	virtual void computeEnergyWithSpeedHolder(const std::deque<double> speedCollector,  struct0_T &vehicleStruct, const double timeStep, const int occupancy);
	virtual void computeTrainEnergyWithSpeed(const double trainMovement, struct0_T &vehicleStruct, const double timeStep); 
	//aa}
	virtual struct0_T initVehicleStruct(const std::string drivetrain);

	/**
	 * Alternative method to compute energy traversed by a Person_MT p within the specified frame_tick number
	 * <p>
	 * This method doesn't return anything, but writes the energy computed into energyModelOutputFile
	 *
	 * @param frameNumber the frame number of the simulation
	 * @param p* pointer to a Person_MT instance that is conducting the trip
	 */
	// void computeEnergyByFrameTick(uint32_t frameNumber, const Person_MT* p);

	/**
	 * Virtual function called by computeEnergyByFrameTick. An instantiation is required for any EnergyModel sub-class
	 *
	 * @param vehicleParams VehicleParams of the vehicle used in the trip
	 * @param segmentStats SegmentStats of the segment being traverse
	 * @return energy amount of energy consumed by the vehicle during this segment
	 */

	std::string getVehicleTypeString(const VehicleParams& vp) const
	{
		std::string vehicleTypeStr; // = trip->getVehicleType();
		switch (vp.getDrivetrain())
		{
		case VehicleParams::ICE : vehicleTypeStr = "ICE"; break;
		case VehicleParams::HEV : vehicleTypeStr = "HEV"; break;
		case VehicleParams::PHEV : vehicleTypeStr = "PHEV"; break;
		case VehicleParams::BEV : vehicleTypeStr = "BEV"; break;
		case VehicleParams::FCV : vehicleTypeStr = "FCV"; break;
		case VehicleParams::CDBUS : vehicleTypeStr = "CDBUS"; break;
		case VehicleParams::HEBUS : vehicleTypeStr = "HEBUS"; break;
		case VehicleParams::TRAIN : vehicleTypeStr = "TRAIN"; break;
		default : vehicleTypeStr = "ICE"; break;
		}
		return vehicleTypeStr;
	}
	/**
	 * gets the output filename for the energy computations
	 *
	 * @return energyModelOutputFile energy model output file
	 */
	std::string getEnergyModelOutputFile() const {
		return energyModelOutputFile;
	}

	/**
	 * Sets the output filename for the energy computations
	 *
	 * @param energyModelOutputFile energy model output file
	 *
	 */
	void setEnergyOutputFile(const std::string energyModelOutputFile) {
		this->energyModelOutputFile = energyModelOutputFile;
	}

	/**
	 * gets the params mapping for the energy model
	 *
	 * @return paramsMapping for energy model
	 */
	const std::map<std::string, std::string>& getParams() const {
		return paramsMapping;
	}

	/**
	 * set the params  for the energy model
	 *
	 * @param key 
	 * @param val
	 */
	std::map<std::string, std::string>& setParams(std::string key,
			std::string val) {
		paramsMapping[key] = val;
	}
//	std::map<std::string, std::string>& setParams(std::string key, std::string val)
//	{
//		paramsMapping[key] = val;
//	}

	/**
	 * gets the model type for the energy model
	 *
	 * @return modelType of the energyModel
	 */
	std::string getModelType() const {
		return modelType;
	}

	void setTripOutput( bool output_trip)
	{
		this->trip_output = output_trip;
	}
	void setSegmentOutput(bool output_segment)
	{
		this->segment_output = output_segment;
	}
	/**
	 * Sets the model type for the energy model
	 *
	 * @param modelType of the energyModel 
	 *
	 */
	void setModelType(const std::string modelType) {
		this->modelType = modelType;
	}

private:
	// type of energy model (e.g. 'simple', 'tripenergy', tripenergSO')
	std::string modelType;
	// name of energy model output file
	std::string energyModelOutputFile;
	// mapping of energy model parameter (coefficient) names => value
	std::map<std::string, std::string> paramsMapping;

  bool trip_output;
  bool segment_output;
};

} // end namespace medium
} // end namespace sim_mob

