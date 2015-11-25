//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/nondet_random.hpp>
#include <boost/random.hpp>
#include <limits>

#include "Driver.hpp"
#include "entities/roles/driver/models/CarFollowModel.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "util/Math.hpp"
#include "util/Utils.hpp"

using std::numeric_limits;
using namespace sim_mob;
using namespace std;

namespace
{

double feet2Unit(double feet)
{
	return feet * 0.158;
}

double millisecondToSecond(double milliSec)
{
	return milliSec / 1000.0;
}

double calculateHeadway(double space, double speed, double elapsedSeconds, double maxAcceleration)
{
	if (speed == 0)
	{
		return -1;
	}
	else
	{
		return 2 * space / (speed + speed + elapsedSeconds * maxAcceleration);
	}
}

}

MITSIM_CF_Model::MITSIM_CF_Model(DriverUpdateParams &params)
{
	modelName = "general_driver_model";
	splitDelimiter = " ,";
	
	readDriverParameters(params);
}

MITSIM_CF_Model::~MITSIM_CF_Model()
{
}

void MITSIM_CF_Model::readDriverParameters(DriverUpdateParams &params)
{
	string speedScalarStr, maxAccStr, decelerationStr, maxAccScaleStr, normalDecScaleStr, maxDecScaleStr;
	string addOn, hBufferUpperStr;
	bool isAMOD = false;

	if (params.driver->getParent()->amodId != "-1")
	{
		isAMOD = true;
	}

	ParameterManager *parameterMgr = ParameterManager::Instance(isAMOD);

	parameterMgr->param(modelName, "speed_scaler", speedScalarStr, string("5 20 20"));
	parameterMgr->param(modelName, "max_acc_car1", maxAccStr, string("10.00  7.90  5.60  4.00  4.00"));
	createSpeedIndices(Vehicle::CAR, speedScalarStr, maxAccStr, maxAccelerationIndex, maxAccUpperBound);
	
	parameterMgr->param(modelName, "max_acceleration_scale", maxAccScaleStr, string("0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5"));
	createScaleIndices(maxAccScaleStr, maxAccelerationScale);

	parameterMgr->param(modelName, "normal_deceleration_car1", decelerationStr, string("7.8 	6.7 	4.8 	4.8 	4.8"));
	createSpeedIndices(Vehicle::CAR, speedScalarStr, decelerationStr, normalDecelerationIndex, normalDecelerationUpperBound);
	
	parameterMgr->param(modelName, "normal_deceleration_scale", normalDecScaleStr, string("1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0"));
	createScaleIndices(maxAccScaleStr, normalDecelerationScale);
	
	parameterMgr->param(modelName, "speed_limit_add_on", addOn, string("-0.1911 -0.0708 -0.0082 0.0397 0.0810 0.1248 0.1661 0.2180 0.2745 0.3657"));
	createScaleIndices(addOn, speedLimitAddon);

	parameterMgr->param(modelName, "Car_following_acceleration_add_on", addOn, string("-1.3564 -0.8547 -0.5562 -0.3178 -0.1036 0.1036 0.3178 0.5562 0.8547 1.3564"));
	Utils::convertStringToArray(addOn, accelerationAddon);

	parameterMgr->param(modelName, "Car_following_deceleration_add_on", addOn, string("-1.3187 -0.8309 -0.5407 -0.3089 -0.1007 0.1007 0.3089 0.5407 0.8309 1.3187"));
	Utils::convertStringToArray(addOn, decelerationAddon);

	parameterMgr->param(modelName, "max_deceleration_car1", decelerationStr, string("-16.0   -14.5   -13.0   -11.0   -9.0"));
	createSpeedIndices(Vehicle::CAR, speedScalarStr, decelerationStr, maxDecelerationIndex, maxDecelerationUpperBound);
	
	parameterMgr->param(modelName, "max_deceleration_scale", maxDecScaleStr, string("1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0"));
	createScaleIndices(maxDecScaleStr, maxDecelerationScale);

	parameterMgr->param(modelName, "acceleration_grade_factor",	accGradeFactor, 0.305);
	parameterMgr->param(modelName, "tmp_all_grades", allGrades, 0.0);

	parameterMgr->param(modelName, "min_speed", minSpeed, 0.1);
	parameterMgr->param(modelName, "min_response_distance", minResponseDistance, 5.0);

	parameterMgr->param(modelName, "yellow_stop_headway", maxYellowLightHeadway, 1.0);
	parameterMgr->param(modelName, "min_speed_yellow", minYellowLightSpeed, 2.2352);

	parameterMgr->param(modelName, "hbuffer_lower", hBufferLower, 0.8);
	parameterMgr->param(modelName, "hbuffer_Upper", hBufferUpperStr, string("1.7498 2.2737 2.5871 2.8379 3.0633 3.2814 3.5068 3.7578 4.0718 4.5979"));
	createScaleIndices(hBufferUpperStr, hBufferUpperScale);
	hBufferUpper = getH_BufferUpperBound();

	string cfParamStr;
	parameterMgr->param(modelName, "CF_parameters_1", cfParamStr, string("0.0400, 0.7220, 0.2420, 0.6820, 0.6000, 0.8250"));
	createCF_Params(cfParamStr, CF_parameters[0]);
	
	parameterMgr->param(modelName, "CF_parameters_2", cfParamStr, string("-0.0418 0.0000 0.1510 0.6840 0.6800 0.8020"));
	createCF_Params(cfParamStr, CF_parameters[1]);

	string targetGapAccParmStr;
	parameterMgr->param(modelName, "target_gap_acc_parm", targetGapAccParmStr, string("0.604, 0.385, 0.323, 0.0678, 0.217,0.583, -0.596, -0.219, 0.0832, -0.170, 1.478, 0.131, 0.300"));
	createScaleIndices(targetGapAccParmStr, targetGapAccParm);

	string updateStepSizeStr;
	parameterMgr->param(modelName, "dec_update_step_size", updateStepSizeStr, string("0.5 0.0 0.5 0.5 0.5"));
	createUpdateSizeParams(updateStepSizeStr, decUpdateStepSize);

	parameterMgr->param(modelName, "speed_factor", speedFactor, 1.0);

	parameterMgr->param(modelName, "acc_update_step_size", updateStepSizeStr, string("1.0 0.0 1.0 1.0 0.5"));
	createUpdateSizeParams(updateStepSizeStr, accUpdateStepSize);

	parameterMgr->param(modelName, "uniform_speed_update_step_size", updateStepSizeStr, string("1.0 0.0 1.0 1.0 0.5"));
	createUpdateSizeParams(updateStepSizeStr, uniformSpeedUpdateStepSize);

	parameterMgr->param(modelName, "stopped_vehicle_update_step_size", updateStepSizeStr, string("0.5 0.0 0.5 0.5 0.5"));
	createUpdateSizeParams(updateStepSizeStr, stoppedUpdateStepSize);

	parameterMgr->param(modelName, "visibility_distance", visibilityDistance, 10.0);

	parameterMgr->param(modelName, "FF_Acc_Params_b2", params.FFAccParamsBeta, 0.3091);

	parameterMgr->param(modelName, "driver_signal_perception_distance", signalVisibilityDist, 75.0);
	
	boost::random_device seed_gen;
	long int seed = seed_gen();
	updateSizeRNG = boost::mt19937(seed);
	
	calcUpdateStepSizes();
	
	//Initialise step size, i = 3 is for stopped vehicle
	params.nextStepSize = updateStepSize[3];
	
	if (params.nextStepSize == 0)
	{
		params.nextStepSize = params.elapsedSeconds;
	}
	
	nextPerceptionSize = perceptionSize[3];
}

void MITSIM_CF_Model::createCF_Params(string &strParams, CarFollowingParams &cfParams)
{
	std::vector<std::string> arrayStr;
	vector<double> params;
	
	boost::trim(strParams);
	boost::split(arrayStr, strParams, boost::is_any_of(splitDelimiter), boost::token_compress_on);
	
	for (int i = 0; i < arrayStr.size(); ++i)
	{
		double res = 0;
		
		try
		{
			res = boost::lexical_cast<double>(arrayStr[i].c_str());
		}
		catch (boost::bad_lexical_cast&)
		{
			std::string str = "Error: Cannot convert <" + strParams + "> to type double";
			throw std::runtime_error(str);
		}
		
		params.push_back(res);
	}
	
	cfParams.alpha = params[0];
	cfParams.beta = params[1];
	cfParams.gama = params[2];
	cfParams.lambda = params[3];
	cfParams.rho = params[4];
	cfParams.stddev = params[5];
}

void MITSIM_CF_Model::createUpdateSizeParams(string &strParams, UpdateStepSizeParam &stepSizeParams)
{
	std::vector<std::string> arrayStr;
	vector<double> params;
	
	boost::trim(strParams);
	boost::split(arrayStr, strParams, boost::is_any_of(splitDelimiter), boost::token_compress_on);
	
	for (int i = 0; i < arrayStr.size(); ++i)
	{
		double res = 0;
		
		try
		{
			res = boost::lexical_cast<double>(arrayStr[i].c_str());
		}
		catch (boost::bad_lexical_cast&)
		{
			std::string str = "Error: Cannot convert <" + strParams + "> to type double";
			throw std::runtime_error(str);
		}
		
		params.push_back(res);
	}
	
	stepSizeParams.mean = params[0];
	stepSizeParams.stdev = params[1];
	stepSizeParams.lower = params[2];
	stepSizeParams.upper = params[3];
	stepSizeParams.perception = params[4];
}

void MITSIM_CF_Model::createScaleIndices(string &data, vector<double> &container)
{
	std::vector<std::string> arrayStr;
	
	boost::trim(data);
	boost::split(arrayStr, data, boost::is_any_of(splitDelimiter), boost::token_compress_on);
	
	for (int i = 0; i < arrayStr.size(); ++i)
	{
		double res = 0;
		
		try
		{
			res = boost::lexical_cast<double>(arrayStr[i].c_str());
		}
		catch (boost::bad_lexical_cast&)
		{
			std::string str = "Error: Cannot convert <" + data + "> to type double";
			throw std::runtime_error(str);
		}
		
		container.push_back(res);
	}
}

void MITSIM_CF_Model::createSpeedIndices(VehicleBase::VehicleType vhType, string &speedScalerStr, string &cstr,
										 map<VehicleBase::VehicleType, map<int, double> > &idx, int &upperBound)
{
	//For example
	//speedScalerStr "5 20 20" ft/sec
	//maxAccStr      "10.00  7.90  5.60  4.00  4.00" ft/(s^2)
	std::vector<std::string> arrayStr;
	
	boost::trim(speedScalerStr);
	boost::split(arrayStr, speedScalerStr, boost::is_any_of(splitDelimiter), boost::token_compress_on);
	
	std::vector<double> speedScalerArrayDouble;
	
	for (int i = 0; i < arrayStr.size(); ++i)
	{
		double res = 0;
		try
		{
			res = boost::lexical_cast<double>(arrayStr[i].c_str());
		}
		catch (boost::bad_lexical_cast&)
		{
			std::string str = "Error: Cannot convert <" + speedScalerStr + "> to type double.";
			throw std::runtime_error(str);
		}
		
		speedScalerArrayDouble.push_back(res);
	}
	
	arrayStr.clear();

	boost::algorithm::trim(cstr);
	boost::split(arrayStr, cstr, boost::is_any_of(splitDelimiter), boost::token_compress_on);
	
	std::vector<double> cArrayDouble;
	
	for (int i = 0; i < arrayStr.size(); ++i)
	{
		double res = 0;
		try
		{
			res = boost::lexical_cast<double>(arrayStr[i].c_str());
		}
		catch (boost::bad_lexical_cast&)
		{
			std::string str = "Error: Cannot convert <" + cstr + "> to type double.";
			throw std::runtime_error(str);
		}
		
		cArrayDouble.push_back(res);
	}

	upperBound = round(speedScalerArrayDouble[1] * (speedScalerArrayDouble[0] - 1));
	
	map<int, double> cIdx;
	
	for (int speed = 0; speed <= upperBound; ++speed)
	{
		// Convert speed value to a table index.
		int j = speed / speedScalerArrayDouble[1];
		double maxAcc = 0;		
		
		if (j >= (speedScalerArrayDouble[0] - 1))
		{
			maxAcc = cArrayDouble[speedScalerArrayDouble[0] - 1];
		}
		else
		{
			maxAcc = cArrayDouble[j];
		}
		
		cIdx.insert(std::make_pair(speed, maxAcc));
	}

	idx[vhType] = cIdx;
}

double MITSIM_CF_Model::getMaxAcceleration(DriverUpdateParams &params, VehicleBase::VehicleType vhType)
{
	int speed = params.perceivedFwdVelocity;
	
	if (speed < 0)
	{
		speed = 0;
	}
	
	if (speed > maxAccUpperBound)
	{
		speed = maxAccUpperBound;
	}

	double maxTableAcc = maxAccelerationIndex[vhType][speed];

	double maxAcc = (maxTableAcc - allGrades * accGradeFactor) * getMaxAccScalar();

	return maxAcc;
}

double MITSIM_CF_Model::getNormalDeceleration(DriverUpdateParams &params, VehicleBase::VehicleType vhType)
{
	int speed = params.perceivedFwdVelocity;
	
	if (speed < 0)
	{
		speed = 0;
	}
	
	if (speed > normalDecelerationUpperBound)
	{
		speed = normalDecelerationUpperBound;
	}

	double normalDec = normalDecelerationIndex[vhType][speed];

	double dec = (normalDec - allGrades * accGradeFactor) * getNormalDecScalar();

	return dec;
}

double MITSIM_CF_Model::getMaxDeceleration(DriverUpdateParams &params, VehicleBase::VehicleType vhType)
{
	int speed = params.perceivedFwdVelocity;
	
	if (speed < 0)
	{
		speed = 0;
	}
	
	if (speed > maxDecelerationUpperBound)
	{
		speed = maxDecelerationUpperBound;
	}

	double maxDec = maxDecelerationIndex[vhType][speed];

	double dec = (maxDec - allGrades * accGradeFactor) * getMaxDecScalar();

	return dec;
}

double MITSIM_CF_Model::getMaxAccScalar()
{
	int scaleNo = Utils::generateInt(1, maxAccelerationScale.size() - 1);
	double res = Utils::generateFloat(maxAccelerationScale[scaleNo - 1], maxAccelerationScale[scaleNo]);
	
	return res;
}

double MITSIM_CF_Model::getNormalDecScalar()
{
	int scaleNo = Utils::generateInt(1, normalDecelerationScale.size() - 1);
	double res = Utils::generateFloat(normalDecelerationScale[scaleNo - 1], normalDecelerationScale[scaleNo]);
	
	return res;
}

double MITSIM_CF_Model::getMaxDecScalar()
{
	int scaleNo = Utils::generateInt(1, maxDecelerationScale.size() - 1);
	double res = Utils::generateFloat(normalDecelerationScale[scaleNo - 1], normalDecelerationScale[scaleNo]);

	return res;
}

double MITSIM_CF_Model::getSpeedLimitAddon()
{
	int scaleNo = Utils::generateInt(1, speedLimitAddon.size() - 1);
	double res = Utils::generateFloat(speedLimitAddon[scaleNo - 1], speedLimitAddon[scaleNo]);
	
	return res;
}

double MITSIM_CF_Model::getAccelerationAddon()
{
	int scaleNo = Utils::generateInt(1, accelerationAddon.size() - 1);
	double res = Utils::generateFloat(accelerationAddon[scaleNo - 1], accelerationAddon[scaleNo]);
	
	return res;
}

double MITSIM_CF_Model::getDecelerationAddon()
{
	int scaleNo = Utils::generateInt(1, decelerationAddon.size() - 1);
	double res = Utils::generateFloat(decelerationAddon[scaleNo - 1], decelerationAddon[scaleNo]);
	
	return res;
}

double MITSIM_CF_Model::getH_BufferUpperBound()
{
	int scaleNo = Utils::generateInt(1, hBufferUpperScale.size() - 1);
	double res = Utils::generateFloat(hBufferUpperScale[scaleNo - 1], hBufferUpperScale[scaleNo]);
	
	return res;
}

double MITSIM_CF_Model::getHeadwayBuffer()
{
	return Utils::generateFloat(hBufferLower, hBufferUpper);
}

double MITSIM_CF_Model::makeAcceleratingDecision(DriverUpdateParams &params)
{
	params.cfDebugStr = "";

	//Calculate the state based variables for the current state
	calcStateBasedVariables(params);
	
	//Calculate the desired speed
	params.desiredSpeed = calcDesiredSpeed(params);

	//Select maximum acceleration by default
	double acceleration = params.maxAcceleration;
	params.accSelect = "max";
	
	//Calculate the different accelerations
	
	double aB = calcMergingAcc(params);
	double aC = calcTrafficSignalAcc(params);
	double aD = calcYieldingAcc(params);
	
	double aE = calcWaitForLaneExitAcc(params);
	//double aF = calcWaitForAllowedLaneAcc(params);
	
	//MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)
	//double aG = calcLaneDropRate(p);
	
	double aH = params.maxAcceleration;
	std::string aHStr = "aH";
	
	//Check if we're in middle of performing lane change
	if (!params.getStatus(STATUS_LC_CHANGING))
	{
		if (params.getStatus(STATUS_ADJACENT))
		{
			aH = calcAdjacentGapRate(params);
			aHStr = "aHA";
		}
		else if (params.getStatus(STATUS_BACKWARD))
		{
			aH = calcBackwardGapAcc(params);
			aHStr = "aHB";
		}
		else if (params.getStatus(STATUS_FORWARD))
		{
			aH = calcForwardGapAcc(params);
			aHStr = "aHF";
		}
		else
		{
			aH = calcDesiredSpeedAcc(params);
			aHStr = "aHD";
		}
	}

	double aZ = calcCarFollowingAcc(params);
	double aSP = calcAccForStoppingPoint(params);
	
	//Choose the minimum acceleration
	
	if(acceleration > aB)
	{
		acceleration = aB;
		params.accSelect = "aB";
	}
	
	if(acceleration > aC)
	{
		acceleration = aC;
		params.accSelect = "aC";
	}

	if (acceleration > aD)
	{
		acceleration = aD;
		params.accSelect = "aD";
	}
	
	if(acceleration > aE)
	{
		acceleration = aE;
		params.accSelect = "aE";
	}
	
	if (acceleration > aH)
	{
		acceleration = aH;
		params.accSelect = aHStr;
	}

	if (acceleration > aC)
	{
		acceleration = aC;
		params.accSelect = "aC";
	}

	if (acceleration > aZ)
	{
		acceleration = aZ;
		params.accSelect = "aZ";
	}

	if (acceleration > aSP)
	{
		acceleration = aSP;
		params.accSelect = "aSP";
	}

	//SEVERAL CONDITONS MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)

	//Update the selected the acceleration (for debugging)
	params.acc = acceleration;

	//If we are braking, alert the follower
	if (acceleration < -ACC_EPSILON)
	{
		//alert the vehicle behind if it is close
		if (params.nvBack.exists())
		{
			Driver *rearDriver = const_cast<Driver*> (params.nvBack.driver);
			DriverUpdateParams &rearDriverParams = rearDriver->getParams();
			
			if (params.nvBack.distance < visibilityDistance && !(rearDriver->IsBusDriver() && rearDriverParams.getStatus(STATUS_STOPPED)))
			{
				float alert = CF_CRITICAL_TIMER_RATIO * updateStepSize[0];
				rearDriverParams.reactionTimeCounter = std::min<double>(alert, rearDriverParams.reactionTimeCounter);
			}
		}
	}

	//Calculate the next step size
	params.reactionTimeCounter = calcNextStepSize(params);
	
	//If we're in emergency regime, reduce the reactionTimeCounter
	if (params.getStatus(STATUS_REGIME_EMERGENCY))
	{
		params.reactionTimeCounter = params.getNextStepSize() * CF_CRITICAL_TIMER_RATIO;
	}
	else
	{
		params.reactionTimeCounter = params.getNextStepSize();
	}

	return acceleration;
}

double MITSIM_CF_Model::calcCarFollowingAcc(DriverUpdateParams &params, NearestVehicle &nearestVehicle)
{
	double res = 0;
	std::stringstream debugStr;

	//Unset status
	params.unsetStatus(STATUS_REGIME_EMERGENCY);
	params.gapBetnVehicles = params.perceivedDistToFwdCar;

	debugStr << "t" << params.now.frame();

	params.headway = 99;
	if (params.perceivedDistToFwdCar == DBL_MAX)
	{
		res = calcFreeFlowingAcc(params, params.desiredSpeed);
		debugStr << "DEF;" << res;
	}
	else
	{
		debugStr << "ELSE;";
		
		//When nearest vehicle is the left/right vehicle, we cannot use perceived values!
		//Create perceived left, right values
		
		params.velocityLeadVehicle = params.perceivedFwdVelocityOfFwdCar;
		params.accLeadVehicle = params.perceivedAccelerationOfFwdCar;

		double dt = params.nextStepSize;
		float auxspeed = params.perceivedFwdVelocity == 0 ? 0.00001 : params.perceivedFwdVelocity;

		float headway = 2.0 * params.gapBetnVehicles / (auxspeed + params.perceivedFwdVelocity);

		debugStr << "+" << headway << "+" << params.gapBetnVehicles << "+" << auxspeed << "+" << params.perceivedFwdVelocity << ";";

		double emergSpace = nearestVehicle.distance;

		debugStr << emergSpace << ";";

		//If we have no space left to move, immediately cut off acceleration.
		if (emergSpace <= params.driver->getVehicleLength())
		{
			double speed = params.perceivedFwdVelocity;
			double emergHeadway = calculateHeadway(emergSpace, speed, params.elapsedSeconds, params.maxAcceleration);
			
			if (emergHeadway < hBufferLower)
			{
				//We need to brake. Override.
				params.gapBetnVehicles = emergSpace;
				headway = emergHeadway;
			}

			debugStr << "EM;" << emergHeadway << ";";
		}

		float v = params.velocityLeadVehicle + params.accLeadVehicle * dt;
		params.spaceStar = params.gapBetnVehicles + 0.5 * (params.velocityLeadVehicle + v) * dt;
		
		if (headway < hBufferLower)
		{
			res = calcEmergencyDeceleration(params);
			params.setStatus(STATUS_REGIME_EMERGENCY);
			debugStr << "LO;";
		}
		
		hBufferUpper = getH_BufferUpperBound();
		
		if (headway > hBufferUpper)
		{
			res = accOfMixOfCFandFF(params, params.desiredSpeed);
			debugStr << "UP;";
		}
		
		if (headway <= hBufferUpper && headway >= hBufferLower)
		{
			res = calcAccOfCarFollowing(params);
			debugStr << "LOUP;";
		}

		params.headway = headway;
	}

	return res;
}

double MITSIM_CF_Model::calcCarFollowingAcc(DriverUpdateParams &params)
{
	double aZ1 = calcCarFollowingAcc(params, params.nvFwd);
	double aZ2 = calcCarFollowingAcc(params, params.nvFwdNextLink);

	return std::min(aZ1, aZ2);
}

double MITSIM_CF_Model::calcMergingAcc(DriverUpdateParams &params)
{
	double acc = params.maxAcceleration;

	//Vehicles from freeways and on ramps have different priority. 
	//Separate procedures are applied. (MITSIM TS_CFModels.cc)
	DriverMovement *driverMvt = dynamic_cast<DriverMovement*> (params.driver->Movement());

	if(!driverMvt->fwdDriverMovement.isInIntersection())
	{
		if (driverMvt->fwdDriverMovement.getCurrLink()->getLinkType() == LINK_TYPE_EXPRESSWAY)
		{
			if (params.nvLeadFreeway.exists())
			{
				double headway = getHeadwayBuffer();

				if (params.nvLeadFreeway.distance < headway)
				{
					//MITSIM TS_CFModels.cc
					acc = params.nvLeadFreeway.driver->getFwdAcceleration()
							+ calcTargetSpeedAcc(params, params.nvLeadFreeway.distance, params.nvLeadFreeway.driver->getFwdVelocity());
				}
			}
		}
		else if (driverMvt->fwdDriverMovement.getCurrLink()->getLinkType() == LINK_TYPE_RAMP)
		{
			if (params.nvLagFreeway.exists())
			{
				if (!isGapAcceptable(params, params.nvLagFreeway))
				{
					// The gap is not acceptable. Prepare to stop.
					return calcBrakeToStopAcc(params, params.distToStop);
				}
			}
			else
			{
				if (params.nvLeadFreeway.exists())
				{
					//Lead vehicle exists, brake to target speed
					acc = calcTargetSpeedAcc(params, params.nvLeadFreeway.distance, params.nvLeadFreeway.driver->getFwdVelocity());
				}
			}
		}
	}

	return acc;
}

bool MITSIM_CF_Model::isGapAcceptable(DriverUpdateParams &params, NearestVehicle &nearestVehicle)
{
	float acceleration = params.maxAcceleration;
	float speedOfOtherVehicle = nearestVehicle.driver->getFwdVelocity();

	Driver *otherDriver = const_cast<Driver *> (nearestVehicle.driver);
	DriverUpdateParams &otherDriverParams = otherDriver->getParams();

	float maxAcc = otherDriverParams.maxAcceleration;
	float speed;

	double dt = params.nextStepSize;

	double distance = params.distToStop;
	double currentSpeed = params.driver->getFwdVelocity();

	if (distance > Math::DOUBLE_EPSILON)
	{
		//maximum speed can be reached at the end of the lane.
		speed = currentSpeed * currentSpeed + 2.0 * acceleration * distance;
		speed = (speed > Math::DOUBLE_EPSILON) ? sqrt(speed) : 0.0;

		//Shortest time required to arrive at the end of the lane
		dt = (speed - currentSpeed) / acceleration;
	}
	else
	{
		speed = currentSpeed;
		dt = 0;
	}

	float gap = nearestVehicle.distance;

	// Speed at the predicted position
	speedOfOtherVehicle += maxAcc * dt;
	
	float sd = (speedOfOtherVehicle - speed) * getHeadwayBuffer();
	float threshold = (sd > 0.0) ? sd : 0.0;

	//Check if the gap is acceptable
	if (gap > threshold)
	{
		return true;
	}
	else
	{
		return false;
	}
}

double MITSIM_CF_Model::calcTrafficSignalAcc(DriverUpdateParams& p)
{
	double minAcc = p.maxAcceleration;
	TrafficColor color = p.perceivedTrafficColor;
	double distanceToTrafficSignal = p.perceivedDistToTrafficSignal;

	if (distanceToTrafficSignal < signalVisibilityDist)
	{
		if (color == TRAFFIC_COLOUR_RED)
		{
			double acc = calcBrakeToStopAcc(p, distanceToTrafficSignal);
			minAcc = std::min(acc, minAcc);
		}
		else if (color == TRAFFIC_COLOUR_AMBER)
		{
			double maxSpeed = (p.perceivedFwdVelocity > minYellowLightSpeed) ? p.perceivedFwdVelocity : minYellowLightSpeed;
			
			if (distanceToTrafficSignal / maxSpeed > maxYellowLightHeadway)
			{
				double acc = calcBrakeToStopAcc(p, distanceToTrafficSignal);
				minAcc = std::min(acc, minAcc);
			}
		}
		else if (color == TRAFFIC_COLOUR_GREEN)
		{
			minAcc = p.maxAcceleration;
		}
	}
	
	return minAcc;
}

double MITSIM_CF_Model::calcYieldingAcc(DriverUpdateParams &params)
{
	float acc = 0;
	params.lcDebugStr << ";---CYR";
	
	if (params.flag(FLAG_YIELDING))
	{
		// This vehicle is yielding to another vehicle
		
		params.lcDebugStr << ";DING";
		
		uint32_t dt_sec = millisecondToSecond(params.now.ms() - params.yieldTime.ms());
		
		params.lcDebugStr << ";dt" << dt_sec;
		
		//Make sure a vehicle will not yield infinitely.
		if (dt_sec > params.lcMaxYieldingTime)
		{
			params.driver->setYieldingToDriver(NULL);
			params.unsetFlag(FLAG_YIELDING);
			params.lcDebugStr << ";yd1";
			return params.maxAcceleration;
		}

		bool rightFwdVhFlag = false;
		
		if (params.nvRightFwd.exists())
		{
			params.lcDebugStr << ";yd2";
			
			Driver* dRF = const_cast<Driver*> (params.nvRightFwd.driver);
			DriverUpdateParams& pRF = dRF->getParams();
			
			if (pRF.flag(FLAG_NOSING_LEFT))
			{
				rightFwdVhFlag = true;
				params.lcDebugStr << ";yd3";
			}
		}

		bool leftFwdVhFlag = false;
		
		if (params.nvLeftFwd.exists())
		{
			params.lcDebugStr << ";yd4";
			
			Driver* d = const_cast<Driver*> (params.nvLeftFwd.driver);
			DriverUpdateParams& p = d->getParams();
			
			if (p.flag(FLAG_NOSING_RIGHT))
			{
				leftFwdVhFlag = true;
				p.lcDebugStr << ";yd5";
			}
		}

		if (params.flag(FLAG_YIELDING_RIGHT))
		{
			params.lcDebugStr << ";yd6";
			
			//Check if the right front vehicle is nosing
			if ((params.rightLane) && (params.nvRightFwd.exists()) && params.nvRightFwd.driver == params.driver->getYieldingToDriver() && rightFwdVhFlag)
			{
				params.lcDebugStr << ";yd7";
				
				acc = calcCarFollowingAcc(params, params.nvRightFwd);
				
				if (acc < params.normalDeceleration)
				{
					acc = params.normalDeceleration;
				}
				else if (acc > 0)
				{
					acc = 0.0;
				}
				
				params.lcDebugStr << ";acc" << acc;
				
				return acc;
			}
		}
		else if (params.flag(FLAG_YIELDING_LEFT))
		{
			params.lcDebugStr << ";yd8";
			
			//Check if the left front vehicle is nosing
			if ((params.leftLane) && (params.nvLeftFwd.exists()) && params.nvLeftFwd.driver == params.driver->getYieldingToDriver() && leftFwdVhFlag)
			{
				params.lcDebugStr << ";yd9";
				
				acc = calcCarFollowingAcc(params, params.nvLeftFwd);
				
				if (acc < params.normalDeceleration)
				{
					acc = params.normalDeceleration;
				}
				else if (acc > 0)
				{
					acc = 0.0;
				}
				
				params.lcDebugStr << ";acc" << acc;
				
				return acc;
			}
		}

		params.driver->setYieldingToDriver(NULL);
		params.unsetFlag(FLAG_YIELDING);
		
		params.lcDebugStr << ";yd10" << acc;
		
		return params.maxAcceleration;
	}
	else if (params.flag(FLAG_NOSING))
	{
		// This vehicle is nosing
		
		params.lcDebugStr << ";SING";
		
		bool rightBackVhFlag = false;
		
		if (params.nvRightBack.exists())
		{
			Driver* d = const_cast<Driver*> (params.nvRightBack.driver);
			DriverUpdateParams& pd = d->getParams();
			
			if (pd.flag(FLAG_YIELDING_LEFT) || pd.flag(FLAG_NOSING))
			{
				rightBackVhFlag = true;
			}
		}
		
		bool leftBackVhFlag = false;
		
		if (params.nvLeftBack.exists())
		{
			Driver* d = const_cast<Driver*> (params.nvLeftBack.driver);
			DriverUpdateParams& pd = d->getParams();
			
			if (pd.flag(FLAG_YIELDING_RIGHT) || pd.flag(FLAG_NOSING))
			{
				leftBackVhFlag = true;
			}
		}

		if (params.flag(FLAG_NOSING_RIGHT))
		{
			params.lcDebugStr << ";RT";
			params.lcDebugStr << ";RBD" << params.nvRightBack.distance;
			params.lcDebugStr << ";RFD" << params.nvRightFwd.distance;
			
			//Check if the right rear vehicle is yielding to the left
			if ((params.rightLane) && (params.nvRightBack.exists()) && rightBackVhFlag)
			{
				acc = calcAccToCreateGap(params, params.nvRightFwd, params.lcMinGap(2) + Math::DOUBLE_EPSILON);
				double res = std::max<double>(params.maxDeceleration, acc);
				
				params.lcDebugStr << ";acc" << acc;
				
				return res;
			}
		}
		else if (params.flag(FLAG_NOSING_LEFT))
		{
			params.lcDebugStr << ";LT";
			
			if ((params.leftLane) && (params.nvLeftBack.exists()) && leftBackVhFlag)
			{
				acc = calcAccToCreateGap(params, params.nvLeftFwd, params.lcMinGap(2) + Math::DOUBLE_EPSILON);
				return std::max<double>(params.maxDeceleration, acc);
			}
		}

		if (params.getStatus(STATUS_CHANGING) && params.flag(FLAG_LC_FAILED_LEAD))
		{
			return params.normalDeceleration;
		}
		else
		{
			return params.maxAcceleration;
		}

	}
	else
	{
		//Currently this vehicle is neither yielding, nor nosing.
		
		params.lcDebugStr << ";NTH";
		
		return params.maxAcceleration;
	}
}

double MITSIM_CF_Model::calcAccToCreateGap(DriverUpdateParams &params, NearestVehicle &nearestVeh, float gap)
{
	if (!nearestVeh.exists())
	{
		//No vehicle ahead, this constraint does not apply
		return params.maxAcceleration;
	}

	//freedom left
	float dx = nearestVeh.distance - gap;
	float dv = params.currSpeed - nearestVeh.driver->getFwdVelocity();
	float dt = params.nextStepSize;
	
	if (dt <= 0.0)
	{
		return params.maxAcceleration;
	}

	if (dx < 0.01 || dv < 0.0)
	{
		//insufficient gap or my speed is slower than the leader
		//front->accRate_ + 2.0 * (dx - dv * dt) / (dt * dt);
		double res = nearestVeh.driver->getFwdAcceleration() + 2.0 * (dx - dv * dt) / (dt * dt);
		return res;
	}
	else
	{
		//gap is ok and my speed is higher.
		//front->accRate_ - 0.5 * dv * dv / dx;
		double res = nearestVeh.driver->getFwdAcceleration() - 0.5 * dv * dv / dx;
		return res;
	}
}

double MITSIM_CF_Model::calcWaitForLaneExitAcc(DriverUpdateParams &params)
{
	double acceleration = params.maxAcceleration;
	DriverMovement *driverMvt = dynamic_cast<DriverMovement*> (params.driver->Movement());
	
	if(!driverMvt->fwdDriverMovement.isInIntersection())
	{
		double dx = driverMvt->fwdDriverMovement.getDistToEndOfCurrLink() - params.driver->getVehicleLength();

		if (dx < params.distanceToNormalStop && params.getStatus(STATUS_CURRENT_LANE_OK) != StatusValue::STATUS_YES)
		{
			acceleration = calcBrakeToStopAcc(params, dx);
		}
	}
	
	return acceleration;
}

double MITSIM_CF_Model::calcWaitForAllowedLaneAcc(DriverUpdateParams &params)
{
	if (params.distToStop < params.distanceToNormalStop && !params.isTargetLane)
	{
		if (params.flag(FLAG_NOSING))
		{
			return params.maxAcceleration;
		}
		
		return calcBrakeToStopAcc(params, params.distanceToNormalStop);
	}
	
	return params.maxAcceleration;
}

double MITSIM_CF_Model::calcDesiredSpeed(DriverUpdateParams &params)
{
	double speedOnSign = 0;
	
	if (params.speedLimit)
	{
		speedOnSign = params.speedLimit;
	}
	else
	{
		speedOnSign = params.maxLaneSpeed;
	}

	float desired = speedFactor * speedOnSign;
	desired = desired * (1 + getSpeedLimitAddon());

	double desiredSpeed = std::min<double>(desired, params.maxLaneSpeed);
	return desiredSpeed;
}

double MITSIM_CF_Model::calcForwardGapAcc(DriverUpdateParams &params)
{
	//Adjacent lead vehicle
	const NearestVehicle *adjVehicle = NULL;

	if (params.getStatus(STATUS_LEFT))
	{
		adjVehicle = &params.nvLeftFwd;
	}
	else if (params.getStatus(STATUS_RIGHT))
	{
		adjVehicle = &params.nvRightFwd;
	}
	else
	{
		// No request for lane change
		return params.maxAcceleration;
	}

	const std::vector<double> &gapAcceptanceParams = targetGapAccParm;
	double distance = 0;
	double dv = 0;
	
	if (adjVehicle->exists())
	{
		distance = adjVehicle->distance + gapAcceptanceParams[0];
		Driver *adjDriver = const_cast<Driver*> (adjVehicle->driver);
		dv = adjDriver->getFwdVelocity() - params.driver->getFwdVelocity();
	}
	else
	{
		distance = adjVehicle->distance + gapAcceptanceParams[0];
		dv = 0 - params.driver->getFwdVelocity();
	}

	float acc = gapAcceptanceParams[1] * pow(distance, gapAcceptanceParams[2]);

	if (dv > 0)
	{
		acc *= pow(dv, gapAcceptanceParams[3]);
	}
	else if (dv < 0)
	{
		acc *= pow(-dv, gapAcceptanceParams[4]);
	}

	acc += getAccelerationAddon() * gapAcceptanceParams[5] / 0.824;
	return acc;
}

double MITSIM_CF_Model::calcBackwardGapAcc(DriverUpdateParams &params)
{
	//Rear vehicle
	const NearestVehicle *rearVehicle = NULL;

	if (params.getStatus(STATUS_LEFT))
	{
		rearVehicle = &params.nvLeftBack;
	}
	else if (params.getStatus(STATUS_RIGHT))
	{
		rearVehicle = &params.nvRightBack;
	}
	else
	{
		// No request for lane change
		return params.maxAcceleration;
	}

	const std::vector<double> &gapAcceptanceParams = targetGapAccParm;

	double distance = 0;
	double dv = 0;

	if (rearVehicle->exists())
	{
		Driver *rearDriver = const_cast<Driver*> (rearVehicle->driver);

		distance = rearVehicle->distance + gapAcceptanceParams[0];
		dv = rearDriver->getFwdVelocity() - params.driver->getFwdVelocity();
	}
	else
	{
		distance = rearVehicle->distance + gapAcceptanceParams[0];
		dv = 0 - params.driver->getFwdVelocity();
	}

	float acc = gapAcceptanceParams[6] * pow(distance, gapAcceptanceParams[7]);

	if (dv > 0)
	{
		acc *= pow(dv, gapAcceptanceParams[8]);
	}
	else if (dv < 0)
	{
		acc *= pow(-dv, gapAcceptanceParams[9]);
	}

	acc += getAccelerationAddon() * gapAcceptanceParams[10] / 0.824;
	return acc;
}

double MITSIM_CF_Model::calcAdjacentGapRate(DriverUpdateParams& p)
{
	//Adjacent vehicle
	const NearestVehicle *adjVehicle = NULL;
	
	//Vehicle behind the adjacent vehicle
	const NearestVehicle * adjRearVehicle = NULL;

	if (p.getStatus(STATUS_LEFT))
	{
		adjVehicle = &p.nvLeftFwd;
		adjRearVehicle = &p.nvLeftBack;
	}
	else if (p.getStatus(STATUS_RIGHT))
	{
		adjVehicle = &p.nvRightFwd;
		adjRearVehicle = &p.nvRightBack;
	}
	else
	{
		// No request for lane change
		return p.maxAcceleration;
	}

	const std::vector<double> &gapAcceptanceParams = targetGapAccParm;

	if (!adjVehicle->exists())
	{
		return p.maxAcceleration;
	}

	if (!adjRearVehicle->exists())
	{
		return p.maxAcceleration;
	}

	Driver *adjDriver = const_cast<Driver*> (adjVehicle->driver);
	Driver *adjRearDriver = const_cast<Driver*> (adjRearVehicle->driver);

	float gap = adjRearDriver->gapDistance(adjDriver);
	float position = adjRearDriver->gapDistance(p.driver) + p.driver->getVehicleLength();
	float acc = gapAcceptanceParams[11] * (gapAcceptanceParams[0] * gap - position);

	acc += getAccelerationAddon() * gapAcceptanceParams[12] / 0.824;
	p.lcDebugStr << "+++acc+++" << acc;
	
	return acc;
}

double MITSIM_CF_Model::calcAccForStoppingPoint(DriverUpdateParams &params)
{
	std::stringstream debugStr;
	debugStr << ";SS-Dist" << params.distanceToStoppingPt << ";" << params.stopPointState << ";";
	
	double acc = params.maxAcceleration;
	
	if (!params.getStatus(STATUS_CHANGING))
	{
		if (params.stopPointState == DriverUpdateParams::ARRIVING_AT_STOP_POINT)
		{
			acc = calcBrakeToStopAcc(params, params.distToStop);
			debugStr << "SP-Arriving;";
			params.cfDebugStr += debugStr.str();
			return acc;
		}
		if (params.stopPointState == DriverUpdateParams::ARRIVED_AT_STOP_POINT || params.stopPointState == DriverUpdateParams::WAITING_AT_STOP_POINT)
		{
			debugStr << "SP-Arrived;";
			acc = params.maxDeceleration;
		}
	}
	
	if (params.stopPointState == DriverUpdateParams::ARRIVED_AT_STOP_POINT && params.perceivedFwdVelocity < 0.1)
	{
		debugStr << "SP-Arrive0;";
		acc = params.maxDeceleration;
		params.stopPointState = DriverUpdateParams::WAITING_AT_STOP_POINT;
		params.stopTimeTimer = params.now.ms();
	}
	
	if (params.stopPointState == DriverUpdateParams::WAITING_AT_STOP_POINT)
	{
		debugStr << "SP-Waiting;";
		double currentTime = params.now.ms();
		double waitTime = millisecondToSecond(currentTime - params.stopTimeTimer);

		debugStr << "SP-wt;" << waitTime;

		if (waitTime > params.currentStopPoint.dwellTime)
		{
			params.stopPointState = DriverUpdateParams::LEAVING_STOP_POINT;
		}
	}
	
	params.cfDebugStr += debugStr.str();
	return acc;
}

double MITSIM_CF_Model::calcBrakeToStopAcc(DriverUpdateParams &params, double distance)
{
	if (distance > Math::DOUBLE_EPSILON)
	{
		double u2 = (params.perceivedFwdVelocity) * (params.perceivedFwdVelocity);
		double acc = -u2 / distance * 0.5;

		if (acc <= params.normalDeceleration)
		{
			return acc;
		}

		double dt = params.nextStepSize;
		double vt = params.perceivedFwdVelocity * dt;
		double a = dt * dt;
		double b = 2.0 * vt - params.normalDeceleration * a;
		double c = u2 + 2.0 * params.normalDeceleration * (distance - vt);
		double d = b * b - 4.0 * a * c;

		if (d < 0 || a <= 0.0)
		{
			return acc;
		}

		return (sqrt(d) - b) / a * 0.5;
	}
	else
	{
		double dt = params.nextStepSize;
		return (dt > 0.0) ?	-params.perceivedFwdVelocity / dt : params.maxDeceleration;
	}
}

double MITSIM_CF_Model::calcDesiredSpeedAcc(DriverUpdateParams &params)
{
	float maxspd = params.maxLaneSpeed;
	double epsilon_v = Math::DOUBLE_EPSILON;
	
	if (params.perceivedFwdVelocity < maxspd - epsilon_v)
	{
		// Use maximum acceleration
		return params.maxAcceleration;
	}
	else if (params.perceivedFwdVelocity > maxspd + epsilon_v)
	{
		// Decelerate
		return params.normalDeceleration;
	}
	else
	{
		// Keep current speed.
		return 0.0;
	}
}

double MITSIM_CF_Model::calcTargetSpeedAcc(DriverUpdateParams &params, double distance, double velocity)
{
	double dt = params.nextStepSize;
	double currentSpeed = params.perceivedFwdVelocity;

	if (distance > Math::DOUBLE_EPSILON)
	{
		float v2 = velocity * velocity;
		float u2 = currentSpeed * currentSpeed;
		float acc = (v2 - u2) / distance * 0.5;

		return acc;
	}
	else
	{
		return (velocity - currentSpeed) / dt;
	}
}

double MITSIM_CF_Model::calcEmergencyDeceleration(DriverUpdateParams &params)
{
	double velocity = params.perceivedFwdVelocity;
	double dv = velocity - params.velocityLeadVehicle;
	double epsilon_v = Math::DOUBLE_EPSILON;
	
	if (velocity < epsilon_v)
	{
		return 0;
	}
	
	double aNormalDec = params.normalDeceleration;
	double a = 0;
	
	if (dv < epsilon_v)
	{
		a = params.accLeadVehicle + 0.25 * aNormalDec;
	}
	else if (params.gapBetnVehicles > 0.01)
	{
		a = params.accLeadVehicle - dv * dv / 2 / params.gapBetnVehicles;
	}
	else
	{
		double dt = params.nextStepSize;
		double s = params.spaceStar;
		double v = params.velocityLeadVehicle + params.accLeadVehicle * dt;
		a = calcTargetSpeedAcc(params, s, v);
	}
	
	return min(params.normalDeceleration, a);
}

double MITSIM_CF_Model::calcAccOfCarFollowing(DriverUpdateParams &params)
{
	double density = params.density;
	double velocity = params.perceivedFwdVelocity;
	int i = (velocity > params.velocityLeadVehicle) ? 1 : 0;
	
	double dv = (velocity > params.velocityLeadVehicle) ? (velocity - params.velocityLeadVehicle) : (params.velocityLeadVehicle - velocity);

	double res = CF_parameters[i].alpha * pow(velocity, CF_parameters[i].beta) / pow(params.nvFwd.distance, CF_parameters[i].gama);
	res *= pow(dv, CF_parameters[i].lambda)	* pow(density, CF_parameters[i].rho);
	res += feet2Unit(Utils::nRandom(0, CF_parameters[i].stddev));

	return res;
}

double MITSIM_CF_Model::calcFreeFlowingAcc(DriverUpdateParams &params, double targetSpeed)
{
	double velocity = params.perceivedFwdVelocity;

	if (velocity < targetSpeed - minSpeed)
	{
		double acc = params.FFAccParamsBeta * (targetSpeed - velocity);
		return acc;
	}
	else if (velocity > targetSpeed + minSpeed)
	{
		return params.normalDeceleration;
	}

	return 0;
}

double MITSIM_CF_Model::accOfMixOfCFandFF(DriverUpdateParams &params, double targetSpeed)
{
	if (params.gapBetnVehicles > params.distanceToNormalStop)
	{
		return calcFreeFlowingAcc(params, targetSpeed);
	}
	else
	{
		double dt = params.nextStepSize;
		double s = params.spaceStar;
		double v = params.velocityLeadVehicle + params.accLeadVehicle * dt;
		return calcTargetSpeedAcc(params, s, v);
	}
}

void MITSIM_CF_Model::calcDistanceForNormalStop(DriverUpdateParams &params)
{
	if (params.perceivedFwdVelocity > minSpeed)
	{
		params.distanceToNormalStop = Math::DOUBLE_EPSILON - 0.5 * params.perceivedFwdVelocity * params.perceivedFwdVelocity / params.normalDeceleration;
		
		if (params.distanceToNormalStop < minResponseDistance)
		{
			params.distanceToNormalStop = minResponseDistance;
		}
	}
	else
	{
		params.distanceToNormalStop = minResponseDistance;
	}
}

void MITSIM_CF_Model::calcStateBasedVariables(DriverUpdateParams &params)
{
	calcDistanceForNormalStop(params);

	// Acceleration rate for a vehicle (a function of vehicle type,
	// facility type, segment grade, current speed).
	params.maxAcceleration = getMaxAcceleration(params);

	// Deceleration rate for a vehicle (a function of vehicle type, and
	// segment grade).
	params.normalDeceleration = getNormalDeceleration(params);

	// Maximum deceleration is function of speed and vehicle class
	params.maxDeceleration = getMaxDeceleration(params);

	// unsetStatus;
	params.unsetStatus(STATUS_REGIME);
}

void MITSIM_CF_Model::calcUpdateStepSizes()
{
	//Deceleration
	double totalReactionTime = sampleFromNormalDistribution(decUpdateStepSize);
	
	//Perception time  = reaction time * perception percentage
	double perceptionTime = totalReactionTime * decUpdateStepSize.perception;

	updateStepSize.push_back(totalReactionTime);
	perceptionSize.push_back(perceptionTime);

	//Acceleration
	totalReactionTime = sampleFromNormalDistribution(accUpdateStepSize);
	
	//Perception time  = reaction time * perception percentage
	perceptionTime = totalReactionTime * accUpdateStepSize.perception;

	updateStepSize.push_back(totalReactionTime);
	perceptionSize.push_back(perceptionTime);

	//Uniform Speed
	totalReactionTime = sampleFromNormalDistribution(uniformSpeedUpdateStepSize);
	
	//Perception time  = reaction time * perception percentage
	perceptionTime = totalReactionTime * uniformSpeedUpdateStepSize.perception;

	updateStepSize.push_back(totalReactionTime);
	perceptionSize.push_back(perceptionTime);

	//Stopped vehicle
	totalReactionTime = sampleFromNormalDistribution(stoppedUpdateStepSize);
	
	//Perception time  = reaction time * perception percentage
	perceptionTime = totalReactionTime * stoppedUpdateStepSize.perception;

	updateStepSize.push_back(totalReactionTime);
	perceptionSize.push_back(perceptionTime);
}

double MITSIM_CF_Model::sampleFromNormalDistribution(UpdateStepSizeParam &stepSizeParams)
{

	if (stepSizeParams.mean == 0)
	{
		return 0;
	}
	
	boost::lognormal_distribution<double> nor(stepSizeParams.mean, stepSizeParams.stdev);
	boost::variate_generator<boost::mt19937, boost::lognormal_distribution<double> > dice(updateSizeRNG, nor);
	double v = dice();
	
	if (v < stepSizeParams.lower)
	{
		return stepSizeParams.lower;
	}
	if (v > stepSizeParams.upper)
	{
		return stepSizeParams.upper;
	}
	
	return v;
}

double CarFollowingModel::calcNextStepSize(DriverUpdateParams &params)
{
	double currAcc = params.driver->getFwdAcceleration();
	double currentSpeed = params.driver->getFwdVelocity();

	int i = 0;
	if (currentSpeed < minSpeed)
	{
		i = 3;
	}
	else
	{
		if (currAcc < -Math::DOUBLE_EPSILON)
		{
			i = 0;
		}
		else if (currAcc > Math::DOUBLE_EPSILON)
		{
			i = 1;
		}
		else if (currentSpeed > Math::DOUBLE_EPSILON)
		{
			i = 2;
		}
		else
		{
			i = 3;
		}
	}

	params.nextStepSize = updateStepSize[i];
	
	if (params.nextStepSize == 0)
	{
		params.nextStepSize = params.elapsedSeconds;
	}
	
	nextPerceptionSize = perceptionSize[i];
	params.driver->resetReactionTime(nextPerceptionSize * 1000);
	
	return params.nextStepSize;
}
