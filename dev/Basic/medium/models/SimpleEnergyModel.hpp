//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * \file SimpleEnergyModel.hpp
 *
 * \author Michael Choi
 */

#pragma once

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>

#include "EnergyModelBase.hpp"

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
class SimpleEnergyModel : public EnergyModelBase
{
public:
	SimpleEnergyModel();
	virtual ~SimpleEnergyModel();

	//Virtual overrides
	virtual void setupParamsVariables();
	//virtual struct0_T initVehicleStruct(TripChainItem* currentSubTrip); //jo Mar13

	//virtual double computeEnergyBySegment(const VehicleParams& vp, const sim_mob::medium::SegmentStats* ss); // jo Mar13
	double computeEnergy(struct0_T& vehicleStruct, const std::vector<double> speed, const double timeStep);

	virtual void computeEnergyWithSpeedHolder(const std::deque<double> speedCollector,  struct0_T &vehicleStruct, const double timeStep, const int occupancy);
	//virtual std::pair<double, double> computeEnergyWithVelocityVector(// const VehicleParams& vp, //jo - Mar13
	//		const std::vector<double> velocityVector, double timeStep, std::string vehicleTypeStr);
	struct0_T initVehicleStruct(const std::string drivetrain);
	//double energyFunction(const VehicleParams::VehicleDriveTrain vdt, const double segLength, const double segVehicleSpeed); //jo Mar13

	// Simple Energy-specific functions
//	std::vector<double> getAccel(const std::vector<int> time_s,  const std::vector<double> speed);
//	double computeMotorForce(double speed_val, double accel_val);
//	std::vector<double> computePower(int N, const std::vector<double> speed, const std::vector<double> accel);
//	std::vector<double> computeEnergy(int N, const std::vector<int> time_s, const std::vector<double> power);
//	double totalEnergy(std::string drivetrain, const std::vector<double> energy);
//	double getPercentSOC( double energy_value );
//	double getFuelUsage( std::string drivetrain, double energy_value );

private:
	// model parameters (will be set by config file)
	float airDensity;
	float rotatingFactor;
	float ICEvehicleMass;
	float gravitationalAcc;
	float drivetrainLoss;
	float carRollingCoefficient;
	float rollingCoefficientC1;
	float rollingCoefficientC2;
	float carFrontalArea;
	float carDragCoefficient;
	float battery_cap;
	float gasoline_GE;
	float economy_HEV;
	float HEVvehicleMass;
	float BEVvehicleMass;
	float CDbusMass;
	float HEbusMass;
	float pspline_alpha;
	float busDragCoefficient;
	float busRollingCoefficient;
	float busFrontalArea;
	float trainDragCoefficient;
	float weightPerRailCarAxle;
	float numberAxlesPerRailCar;
	float totalTrainMassTons;
	float trainRegenEfficiencyParameter;
	float maxHeadEndPower;
	float numberCarsPerTrain;
	float averagePassengerMass;
	float smoothingAlphaICE;
	float smoothingAlphaHEV;
	float smoothingAlphaBEV;
	float smoothingAlphaCDBUS;
	float smoothingAlphaHEBUS;
	float smoothingAlphaTRAIN;
	float maxSpeed;
};

} // end namespace medium
} // end namespace sim_mob

