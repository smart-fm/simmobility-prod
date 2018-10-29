//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "TripEnergyModel.hpp"
#include "../config/ParseMidTermConfigFile.hpp"

using namespace std;
using namespace sim_mob;
using namespace sim_mob::medium;
//struct0_T vehs;
struct0_T *vehs = new struct0_T[10694];
//struct0_T *buses = new struct0_T[8];
//struct0_T vehs_holder[3804];
//struct0_T vehs = vehs_holder[0];

TripEnergyModel::TripEnergyModel() :
		EnergyModelBase() {
	loadDrivingSO(moments, spdBins);
	loadVehicles(vehs);
	std::cout << "Energy Databases Initialized\n";
}

TripEnergyModel::~TripEnergyModel()
{
delete[] vehs;
}

// Necessary in order not to make class abstract
void TripEnergyModel::setupParamsVariables()
{
}


std::pair<double,double> TripEnergyModel::computeEnergyWithVelocityVector(// const VehicleParams& vp, //jo Mar13
		const std::vector<double> velocityVector, double timeStep, std::string vehicleTypeStr)
{
	return std::make_pair(0,0);
}

void TripEnergyModel::computeTrainEnergyWithSpeed(const double trainMovement, struct0_T &vehicleStruct, const double timeStep)
{

}

 

void TripEnergyModel::computeEnergyWithSpeedHolder(const std::deque<double> speedCollector, struct0_T &vehicleStruct, const double timeStep) {
	double j2gal = 131760000;
	double maxAccel = 2.5;
	double output;

	double dV1 = speedCollector[1] - speedCollector[0];
	double dV2 = speedCollector[2] - speedCollector[1];
	double Accel1 = dV1/timeStep;
	double Accel2 = dV2/timeStep;
	double temp[] = {speedCollector[0], speedCollector[1], speedCollector[2]};
	if ( Accel1 > maxAccel) {
		temp[0] += dV1/3;
		temp[1] += -dV1/3;
	}
	else if (-Accel1 > maxAccel) { 
		temp[0] += dV1/3;
		temp[1] += -dV1/3;
	}
	if ( Accel2 > maxAccel) {
		temp[1] += dV2/3;
		temp[2] += -dV2/3;
	}
	else if (-Accel2 > maxAccel) { 
		temp[1] += dV2/3;
		temp[2] += -dV2/3;
	}

	const double smoothedSpeed[] = {temp[0], temp[1], temp[2]}; 

	double Temperature = 18.0;
	so_script(smoothedSpeed, moments, spdBins, &vehicleStruct,Temperature);
}


struct0_T TripEnergyModel::initVehicleStruct(const std::string drivetrain) {
	int vehicleTypeInt = 22;
	if (drivetrain == "Bus") 
	{
		return vehs[10687]; 
	} 
	else if (drivetrain == "Train")
	{
		struct0_T output;
		output.a = 500.0;
		output.tripTotalEnergy = 0;
		output.powertrain = 10;
		output.m = 6*20000;
		return output;
	}
	else if (drivetrain == "NONE")
	{
		return vehs[22];
	}
	else 
	{
		try
		{
			vehicleTypeInt = std::stoi(drivetrain);
		}
		catch (...)
		{
			Warn() << "Bad vehicle type, defaulting to car #22" << std::endl;
			vehicleTypeInt = 22;
		}
		if (vehicleTypeInt < 0) {
		//	std::cout << "Negative Vehicle Type Created, Returning 22" << std::endl;
			return vehs[22];
	
		} else if (vehicleTypeInt < 10685) {
			return vehs[vehicleTypeInt];
		} else {
			std::cout << "Undefined Vehicle Type!" << std::endl;
			return vehs[22];
		}
	}
}
