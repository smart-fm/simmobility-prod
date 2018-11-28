//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "SimpleEnergyModel.hpp"
//#include <SPLINTER/bsplinebuilder.h>
#include <behavioral/params/PersonParams.hpp>

//using namespace SPLINTER;
using namespace std;
using namespace sim_mob;
using namespace sim_mob::medium;

float ETA_DRIVELINE = 0.85;
float ETA_BATTERY = 0.90;
float ETA_MOTOR_BEV = 0.92;
float ETA_DRIVE_BEV = 0.91;
//double ETA_REGEN_BEV = ;
SimpleEnergyModel::SimpleEnergyModel() : EnergyModelBase()
{
}

SimpleEnergyModel::~SimpleEnergyModel()
{
}

void SimpleEnergyModel::setupParamsVariables() {
	std::map<std::string, std::string> params = EnergyModelBase::getParams();
	airDensity = atof(params["air_density"].c_str());
	rotatingFactor = atof(params["rotating_factor"].c_str());
	ICEvehicleMass = atof(params["ICE_vehicle_mass"].c_str());
	gravitationalAcc = atof(params["gravitational_acc"].c_str());
	drivetrainLoss = atof(params["drivetrain_loss"].c_str());
	carRollingCoefficient = atof(params["car_rolling_coefficient"].c_str());
	rollingCoefficientC1 = atof(params["rolling_coefficient_c1"].c_str());
	rollingCoefficientC2 = atof(params["rolling_coefficient_c2"].c_str());
	carFrontalArea = atof(params["car_frontal_area"].c_str());
	carDragCoefficient = atof(params["car_drag_coefficient"].c_str());
	battery_cap = atof(params["battery_cap"].c_str());
	gasoline_GE = atof(params["gasoline_GE"].c_str());
	economy_HEV = atof(params["economy_HEV"].c_str());

	pspline_alpha = atof(params["pspline_alpha"].c_str());

	HEVvehicleMass = atof(params["HEV_vehicle_mass"].c_str());
	BEVvehicleMass = atof(params["BEV_vehicle_mass"].c_str());
	CDbusMass = atof(params["CD_bus_mass"].c_str());
	HEbusMass = atof(params["HE_bus_mass"].c_str());

	busDragCoefficient = atof(params["bus_drag_coefficient"].c_str());
	busFrontalArea = atof(params["bus_frontal_area"].c_str());
	busRollingCoefficient = atof(params["bus_rolling_coefficient"].c_str());

	trainDragCoefficient = atof(params["train_drag_coefficient"].c_str());
	totalTrainMassTons = atof(params["total_train_mass_tons"].c_str());
	trainRegenEfficiencyParameter = atof(params["train_regen_efficiency_parameter"].c_str());
	maxHeadEndPower = atof(params["max_head_end_power"].c_str());
	numberAxlesPerRailCar = atof(params["number_axles_per_rail_car"].c_str());
	weightPerRailCarAxle = atof(params["weight_per_rail_car_axle"].c_str());
	numberCarsPerTrain = atof(params["number_cars_per_train"].c_str());
	averagePassengerMass = atof(params["average_passenger_mass"].c_str());

	smoothingAlphaICE = atof(params["smoothing_alpha_ICE"].c_str());
	smoothingAlphaHEV = atof(params["smoothing_alpha_HEV"].c_str());
	smoothingAlphaBEV = atof(params["smoothing_alpha_BEV"].c_str());
	smoothingAlphaCDBUS = atof(params["smoothing_alpha_CDBUS"].c_str());
	smoothingAlphaHEBUS = atof(params["smoothing_alpha_HEBUS"].c_str());

	maxSpeed = atof(params["max_speed_meters_per_second"].c_str());
}

// Source: http://stackoverflow.com/a/478960
std::string exec(const char* cmd) {
	char buffer[128];
	std::string result = "";
	FILE* pipe = popen(cmd, "r");
	if (!pipe) throw std::runtime_error("popen() failed!");
	try {
		while (!feof(pipe)) {
			if (fgets(buffer, 128, pipe) != NULL)
				result += buffer;
		}
	} catch (...) {
		pclose(pipe);
		throw;
	}
	pclose(pipe);
	return result;
}

// For segment energy computation; not used for now - jo Mar13
//double SimpleEnergyModel::energyFunction(const sim_mob::VehicleParams::VehicleDriveTrain vdt, const double segLength, const double segVehicleSpeed) {
//	return segLength*segVehicleSpeed;
//}
//
//double SimpleEnergyModel::computeEnergyBySegment(const VehicleParams& vp, const sim_mob::medium::SegmentStats* ss) {
//	return energyFunction(vp.getDrivetrain(), ss->getLength(), ss->getSegSpeed(true));
//}

double SimpleEnergyModel::computeEnergy(struct0_T& vehicleStruct, const std::vector<double> speed, const double timeStep)
{
	const int N = 3; //speed.size();
	std::vector<double> accel(N) ;
	std::vector<double> power(N) ;
	std::vector<double> motorForce(N) ;
	std::vector<double> fuelConsumptionRate(N) ;

	int occupancy = vehicleStruct.occup ;



	const double alpha_0_ICE = 5.66*pow(10,-4);
	const double alpha_1_ICE = 4.64*pow(10,-5);
	const double alpha_2_ICE = 1.0*pow(10,-6); // (0.00056575490781253393, 4.6435314306651986e-05, 1.0000000000000002e-06)

	const double alpha_0_CDBUS = 1.66*pow(10,-3);	//19XX series, J. Wang 2017
	const double alpha_1_CDBUS = 8.68*pow(10,-5);	//19XX series, J. Wang 2017

	const double alpha_0_HEBUS = 1.0*pow(10,-3);			//601X series, J. Wang 2017
	const double alpha_1_HEBUS = 5.18*pow(10,-5);	//601X series, J. Wang 2017

	const double alpha_2_BUS = 1.0*pow(10,-8);

	const float headEndPower = 0.33*maxHeadEndPower ; // at normal HEP operating level for train, 1/3 of maxHEP is assumed
	double gasolineConsumed = 0.0;
	double dieselConsumed = 0.0;
	double energyConsumedKWh = 0.0;

	const float gasolineToKWhConversion = 9.1 ; // https://deepresource.wordpress.com/2012/04/23/energy-related-conversion-factors/
	const float dieselToKWhConversion = 10.0  ; // https://deepresource.wordpress.com/2012/04/23/energy-related-conversion-factors/

	accel[0] = 0.0 ;
	power[0] = 0.0 ;

	for (int i=1; i<N; ++i)
	{
		accel[i] = (speed[i] - speed[i-1] )/ (double)timeStep ;
	}

	switch(vehicleStruct.powertrainSimple)
	{
		case 1: // ICEV powertrain, VT-CPFM1 model, Rakha et al 2011
		{
			for (int i=1; i<N; ++i)
			{
				motorForce[i] = (0.5*airDensity*carDragCoefficient*carFrontalArea*speed[i]*speed[i]) +
								0.001*carRollingCoefficient*ICEvehicleMass*gravitationalAcc*(rollingCoefficientC1*speed[i] + rollingCoefficientC2) +
									(1.0 + rotatingFactor)*ICEvehicleMass*accel[i];

				if (motorForce[i] >= 0)
				{
					// fuel consumption rate (gasoline liters/second) is computed with the assumption of power in kW, hence the 0.001 scaling of power
					power[i] = (1.0/drivetrainLoss)*motorForce[i]*speed[i] * 0.001;
					fuelConsumptionRate[i] = alpha_0_ICE + (alpha_1_ICE * power[i]) + (alpha_2_ICE * power[i] * power[i]) ;
				}
				else // if force is less than zero
				{
					//power[i] = drivetrain_loss*motorForce[i]*speed[i]; // no need to compute this
					fuelConsumptionRate[i] = alpha_0_ICE;
				}
				// compute total fuel consumed (liters)
			}
			gasolineConsumed = (smoothingAlphaICE * fuelConsumptionRate[2] + (1 - smoothingAlphaICE) * fuelConsumptionRate[1] ) * (double)timeStep;
			energyConsumedKWh = gasolineConsumed * gasolineToKWhConversion ;
			break ;
		}
		case 2: // HEV powertrain, VT-CPFM model, assume half the gasoline consumption of ICE
		case 3: // PHEV
		{
			for (int i=1; i<N; ++i)
			{
				motorForce[i] = 0.5*airDensity*carDragCoefficient*carFrontalArea*speed[i]*speed[i] +
								0.001*carRollingCoefficient*HEVvehicleMass*gravitationalAcc*(rollingCoefficientC1*speed[i] + rollingCoefficientC2) +
									(1.0 + rotatingFactor)*HEVvehicleMass*accel[i];

				if (motorForce[i] >= 0)
				{
					// fuel consumption rate (gasoline liters/second) is computed with the assumption of power in kW, hence the 0.001 scaling of power
					power[i] = (1.0/drivetrainLoss)*motorForce[i]*speed[i] * 0.001;
					fuelConsumptionRate[i] = alpha_0_ICE + (alpha_1_ICE * power[i]) + (alpha_2_ICE * power[i] * power[i]) ;
				}
				else // if force is less than zero
				{
					//power[i] = drivetrain_loss*motorForce[i]*speed[i]; // no need to compute this
					fuelConsumptionRate[i] = alpha_0_ICE;
				}
				// compute total fuel consumed (liters)
			}
			gasolineConsumed = 0.5 * (smoothingAlphaHEV * fuelConsumptionRate[2] + (1 - smoothingAlphaHEV) * fuelConsumptionRate[1] ) * (double)timeStep;
			energyConsumedKWh = gasolineConsumed * gasolineToKWhConversion ;
			break ;
		}
		case 4: // BEV powertrain, VT-CPEM model, Fiori et al 2016
		case 5: // assume same for FCEV powertrain
		{
			double lossFactorDriveBEV = ETA_BATTERY/(ETA_MOTOR_BEV*ETA_DRIVE_BEV) ;
			for (int i=1; i<N; ++i)
			{
				motorForce[i] = 0.5*airDensity*carDragCoefficient*carFrontalArea*speed[i]*speed[i] +
								0.001*carRollingCoefficient*BEVvehicleMass*gravitationalAcc*(rollingCoefficientC1*speed[i] + rollingCoefficientC2) +
									BEVvehicleMass*accel[i];

				if (motorForce[i] >= 0)
				{
					power[i] = lossFactorDriveBEV*motorForce[i]*speed[i] * 0.001;
				}
				else // if force is less than zero
				{
					double regenFactorBEV = 1.0/(exp(0.0411/abs(accel[i])));
					power[i] = regenFactorBEV*lossFactorDriveBEV*motorForce[i]*speed[i] * 0.001;
				}
			}
			energyConsumedKWh = (smoothingAlphaBEV * power[2] + (1 - smoothingAlphaBEV) * power[1]) * (timeStep/3600.0);
			break;
		}
		case 6: // CDBUS (conventional diesel bus)
		{
			float totalMass = averagePassengerMass * occupancy + CDbusMass;
			for (int i=1; i<N; ++i)
			{
				motorForce[i] = 0.5*airDensity*busDragCoefficient*busFrontalArea*speed[i]*speed[i] +
								0.001*busRollingCoefficient*totalMass*gravitationalAcc*(rollingCoefficientC1*speed[i] + rollingCoefficientC2) +
									(1.0 + rotatingFactor)*totalMass*accel[i];
				if (motorForce[i] >= 0)
				{
					power[i] = (1.0/drivetrainLoss) * motorForce[i] * speed[i] * 0.001 ;
					fuelConsumptionRate[i] = alpha_0_CDBUS + (alpha_1_CDBUS * power[i]) + (alpha_2_BUS * power[i] * power[i]);
				}
				else
				{
					fuelConsumptionRate[i] = alpha_0_CDBUS;
				}

			}
			dieselConsumed = (smoothingAlphaCDBUS * fuelConsumptionRate[2] + (1 - smoothingAlphaCDBUS) * fuelConsumptionRate[1] ) * timeStep;
			energyConsumedKWh = dieselConsumed * dieselToKWhConversion ;
			break;
		}
		case 7: // HEBUS (conventional diesel bus)
		{
			float totalMass = averagePassengerMass * occupancy + HEbusMass;
			for (int i=1; i<N; ++i)
			{
				motorForce[i] = 0.5*airDensity*busDragCoefficient*busFrontalArea*speed[i]*speed[i] +
								0.001*busRollingCoefficient*totalMass*gravitationalAcc*(rollingCoefficientC1*speed[i] + rollingCoefficientC2) +
									(1.0 + rotatingFactor)*totalMass*accel[i];
				if (motorForce[i] >= 0)
				{
					power[i] = (1.0/drivetrainLoss) * motorForce[i] * speed[i] * 0.001 ;
					fuelConsumptionRate[i] = alpha_0_HEBUS + (alpha_1_HEBUS * power[i]) + (alpha_2_BUS * power[i] * power[i]);
				}
				else
				{
					fuelConsumptionRate[i] = alpha_0_CDBUS;
				}
			}
			dieselConsumed = (smoothingAlphaHEBUS * fuelConsumptionRate[2] + (1 - smoothingAlphaHEBUS) * fuelConsumptionRate[1] ) * timeStep;
			energyConsumedKWh = dieselConsumed * dieselToKWhConversion ;
			break;
		}
		case 8: // TRAIN (assumed to be electric) Wang & Rakha model
		{
			// Convert passenger mass to tons
			float totalTrainMassTons = weightPerRailCarAxle * numberAxlesPerRailCar * numberCarsPerTrain + (occupancy * averagePassengerMass * 0.00110231);
			for (int i=1; i<N; ++i)
			{
				// speeds are multiplied by 3.6 to convert from m/s to km/hr for this equation
				// except in the case where distance in meters is required
				// motorForce is given in N
				if (speed[i] == 0)
				{
					motorForce[i] = 0;
				}
				else
				{
					motorForce[i] = (0.6 + 20.0/weightPerRailCarAxle + 0.01*3.6*speed[i]/1.61
							+ (trainDragCoefficient*(3.6*speed[i]/1.61)*(3.6*speed[i]/1.61))/(weightPerRailCarAxle * numberAxlesPerRailCar) //+20*grade
							+ 70*(3.6*speed[i]*3.6*speed[i] - 3.6*speed[i-1]*3.6*speed[i-1])/(8.4 * speed[i] ))
							* totalTrainMassTons * 4.4482;
				}
				if (motorForce[i] >= 0)
				{
					power[i] = headEndPower + motorForce[i]*3.6*speed[i]/(375 * 1.61) * 0.746;
				}
				else
				{
					power[i] = headEndPower + (motorForce[i]*3.6*speed[i]/(375 * 1.61) * 0.746)/(exp(trainRegenEfficiencyParameter/abs(accel[i])));
				}
			}
			energyConsumedKWh = (smoothingAlphaTRAIN * power[2] + (1 - smoothingAlphaTRAIN) * power[1]) * ( timeStep/3600.0);
			break;
		}
	}
	accel.clear();
	power.clear();
	motorForce.clear();
	fuelConsumptionRate.clear();
	return energyConsumedKWh;
}

struct0_T SimpleEnergyModel::initVehicleStruct(const std::string drivetrain)
{
	struct0_T output;
	//output.a =0.0; // not used in Simple Energy
	output.tripTotalEnergy = 0.0;
	//output.m = 0.0;
	output.occup = 0;

	if (drivetrain == "HEV")
	{
		output.powertrainSimple = 2;
	}
	else if (drivetrain == "PHEV")
	{
		output.powertrainSimple = 3;
	}
	else if (drivetrain == "BEV")
	{
		output.powertrainSimple = 4;
	}
	else if (drivetrain == "FCV")
	{
		output.powertrainSimple = 5;
	}
	else if(drivetrain == "ICE")
	{
		output.powertrainSimple = 1;
	}
	else if(drivetrain == "CDBUS")
	{
		output.powertrainSimple = 6; //jo - Bus
	}
	else if(drivetrain == "HEBUS")
	{
		output.powertrainSimple = 7; //jo - Bus
	}
	else if(drivetrain == "TRAIN")
	{
		output.powertrainSimple = 8;
	}
	else
	{
		output.powertrainSimple = 1;
	}
	return output;
}

void SimpleEnergyModel::computeEnergyWithSpeedHolder(const std::deque<double> speedCollector, struct0_T &vehicleStruct, const double timeStep,
		const int occupancy)
{
//	double j2gal = 131760000;
	double maxAccel = 2.5;
//	double maxSpeed = 30.0;
//	double output;

//	VehicleParams vp;

	std::string vehicleTypeStr;
/*	VehicleParams vp = p->getPersonInfoNotConst().getVehicleParams();
	switch (vp.getDrivetrain())
	{
	case VehicleParams::ICE : vehicleTypeStr = "ICE"; break;
	case VehicleParams::HEV : vehicleTypeStr = "HEV"; break;
	case VehicleParams::PHEV : vehicleTypeStr = "PHEV"; break;
	case VehicleParams::BEV : vehicleTypeStr = "BEV"; break;
	case VehicleParams::FCV : vehicleTypeStr = "FCV"; break;
	default : vehicleTypeStr = "ICE"; break;
	}

	if (vehicleStruct.powertrain == 1)
	{
		vehicleTypeStr = "ICE";
	}*/

	double dV1 = speedCollector[1] - speedCollector[0];
	double dV2 = speedCollector[2] - speedCollector[1];
	double Accel1 = dV1/timeStep;
	double Accel2 = dV2/timeStep;
	double temp[] = {speedCollector[0], speedCollector[1], speedCollector[2]};
	if ( Accel1 > maxAccel)
	{
		temp[0] += dV1/3;
		temp[1] += -dV1/3;
	}
	else if (-Accel1 > maxAccel)
	{
		temp[0] += dV1/3;
		temp[1] += -dV1/3;
	}
	if ( Accel2 > maxAccel)
	{
		temp[1] += dV2/3;
		temp[2] += -dV2/3;
	}
	else if (-Accel2 > maxAccel) {
		temp[1] += dV2/3;
		temp[2] += -dV2/3;
	}

	int N = 3;
	std::vector<double> smoothedSpeed(N);

	for (int i = 0; i < N; i++)
	{
		if (temp[i] > maxSpeed)
		{
			temp[i] = maxSpeed;
		}
		smoothedSpeed[i] = temp[i];
	}

	// Set m as occupancy if passed for bus or train
	vehicleStruct.occup = occupancy;
	double total_energy = computeEnergy(vehicleStruct, smoothedSpeed, timeStep) ;

	//double gasoline_gallon_equivalent = getFuelUsage(vehicleTypeStr, total_energy);
	vehicleStruct.tripTotalEnergy += total_energy;

}

//double SimpleEnergyModel::getPercentSOC( double energy_value )
//{
//	double SOC ;
//	SOC = energy_value/( 3600*battery_cap ) ;
//	SOC = SOC * 100 ;
//	return SOC ;
//}

//std::pair<double,double> SimpleEnergyModel::computeEnergyWithVelocityVector( // const VehicleParams& vp, //jo Mar13
//		const std::vector<double> velocityVector, double timeStep, std::string vehicleTypeStr)
//{
//	// VehicleParams::VehicleDriveTrain vdt = vp.getDrivetrain(); //jo Mar13
//	std::string vdt = vehicleTypeStr;
//	if (velocityVector.size() < 4)
//	{
//		Warn() << "Only " << velocityVector.size() <<
//				" unique interpolation points are given. A minimum of degree+1 = 4 unique points are required to build a B-spline basis of degree 3.\n";
//		return std::make_pair(0,0);
//	}
//	// std::cout << "USING SIMPLE MODEL";
//	// std::cout << "\n";
//
//	// Create new DataTable to manage samples
//	DataTable samples;
//	DenseVector x(1);
//	for (int i = 0; i < velocityVector.size(); i++)
//	{
//		x(0) = i*timeStep;
//		samples.addSample(x, velocityVector[i]);
//	}
//
//	BSpline pspline = BSpline::Builder(samples)
//	.degree(3)
//	.smoothing(BSpline::Smoothing::PSPLINE)
//	.alpha(pspline_alpha)
//	.build();
//
//	int N = velocityVector.size()*timeStep;
//	std::vector<double> smoothVV(N);
//	std::vector<int> time_s(N);
//	for (int i = 0; i < N; i++)
//	{
//		x(0) = i;
//		time_s[i] = i;
//		smoothVV[i] = pspline.eval(x);
//	}
//
//	std::vector<double> accel = getAccel(time_s, smoothVV) ;
//	std::vector<double> power = computePower( N, smoothVV, accel) ;
//	std::vector<double> energy = computeEnergy( N, time_s, power) ;
//
//	double total_energy = totalEnergy(vdt, energy) ;
//
//	// WE ignore usage for now - jo Mar13
////	double usage ;
////	if ( vdt==VehicleParams::BEV ) {
////		usage = getPercentSOC( total_energy ) ;
////	}
////	else {
////		usage = getFuelUsage( vdt, total_energy ) ;
////	}
//	double gasoline_gallon_equivalent = getFuelUsage(vdt, total_energy);
// 	return std::make_pair(total_energy, gasoline_gallon_equivalent);
//
//}
