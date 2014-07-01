//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * CF_Model.cpp
 *
 *  Created on: 2011-8-15
 *      Author: wangxy & Li Zhemin
 */

#include <boost/random.hpp>
#include <boost/nondet_random.hpp>
#include <limits>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "entities/vehicle/Vehicle.hpp"
#include "entities/models/CarFollowModel.hpp"
#include "util/Math.hpp"
#include "util/Utils.hpp"
#include "Driver.hpp"

using std::numeric_limits;
using namespace sim_mob;
using namespace std;

namespace {
//Random number generator
//TODO: We need a policy on who can get a generator and why.
//boost::mt19937 gen;

//Threshold defaults
//const double hBufferUpper			=	  1.6;	 ///< upper threshold of headway
//const double hBufferLower			=	  0.8;	 ///< lower threshold of headway

//Set default data for acceleration
//const double maxAcceleration = 5.0;   ///< 5m/s*s, might be tunable later
//const double normalDeceleration = -maxAcceleration*0.6;
//const double maxDeceleration = -maxAcceleration;

//Simple conversion
double feet2Unit(double feet) {
	return feet * 0.158;
}

////Simple struct to hold Car Following model parameters
//struct CarFollowParam {
//	double alpha;
//	double beta;
//	double gama;
//	double lambda;
//	double rho;
//	double stddev;
//};

////Car following parameters for this model.
//const CarFollowParam CF_parameters[2] = {
////    alpha   beta    gama    lambda  rho     stddev
//	{ 0.0400, 0.7220, 0.2420, 0.6820, 0.6000, 0.8250},
//	{-0.0418, 0.0000, 0.1510, 0.6840, 0.6800, 0.8020}
//};

//const double targetGapAccParm[] = {0.604, 0.385, 0.323, 0.0678, 0.217,
//		0.583, -0.596, -0.219, 0.0832, -0.170, 1.478, 0.131, 0.300};

////Acceleration mode// CLA@04/2014 this enum can be deleted
//enum ACCEL_MODE {
//	AM_VEHICLE = 0,
//	AM_PEDESTRIAN = 1,
//	AM_TRAFF_LIGHT = 2,
//	AM_NONE = 3
//};

//double uRandom(boost::mt19937& gen) {
//	boost::uniform_int<> dist(0, RAND_MAX);
//	long int seed_ = dist(gen);
//
//	const long int M = 2147483647; // M = modulus (2^31)
//	const long int A = 48271; // A = multiplier (was 16807)
//	const long int Q = M / A;
//	const long int R = M % A;
//	seed_ = A * (seed_ % Q) - R * (seed_ / Q);
//	seed_ = (seed_ > 0) ? (seed_) : (seed_ + M);
//	return (double) seed_ / (double) M;
//}
//
//double nRandom(boost::mt19937& gen, double mean, double stddev) {
//	double r1 = uRandom(gen), r2 = uRandom(gen);
//	double r = -2.0 * log(r1);
//	if (r > 0.0)
//		return (mean + stddev * sqrt(r) * sin(2 * 3.1415926 * r2));
//	else
//		return (mean);
//}

double CalcHeadway(double space, double speed, double elapsedSeconds,
		double maxAcceleration) {
	if (speed == 0) {
		return 2 * space * 100000;
	} else {
		return 2 * space / (speed + speed + elapsedSeconds * maxAcceleration);
	}
}

} //End anon namespace

/*
 *--------------------------------------------------------------------
 * The acceleration model calculates the acceleration rate based on
 * interaction with other vehicles. The function returns a the
 * most restrictive acceleration (deceleration if negative) rate
 * among the rates given by several constraints.
 *--------------------------------------------------------------------
 */
sim_mob::MITSIM_CF_Model::MITSIM_CF_Model(sim_mob::DriverUpdateParams& p)
//	:cftimer(0.0)
		{
	modelName = "general_driver_model";
	splitDelimiter = " ,";
	initParam(p);
}
void sim_mob::MITSIM_CF_Model::initParam(sim_mob::DriverUpdateParams& p) {
	// speed scaler
	string speedScalerStr, maxAccStr, decelerationStr, maxAccScaleStr,
			normalDecScaleStr, maxDecScaleStr;
	ParameterManager::Instance()->param(modelName, "speed_scaler",
			speedScalerStr, string("5 20 20"));
	// max acceleration
	ParameterManager::Instance()->param(modelName, "max_acc_car1", maxAccStr,
			string("10.00  7.90  5.60  4.00  4.00"));
	makeSpeedIndex(Vehicle::CAR, speedScalerStr, maxAccStr, maxAccIndex,
			maxAccUpperBound);
	ParameterManager::Instance()->param(modelName, "max_acceleration_scale",
			maxAccScaleStr, string("0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5"));
	makeScaleIdx(maxAccScaleStr, maxAccScale);
	// normal deceleration
	ParameterManager::Instance()->param(modelName, "normal_deceleration_car1",
			decelerationStr, string("7.8 	6.7 	4.8 	4.8 	4.8"));
	makeSpeedIndex(Vehicle::CAR, speedScalerStr, decelerationStr,
			normalDecelerationIndex, normalDecelerationUpperBound);
	ParameterManager::Instance()->param(modelName, "normal_deceleration_scale",
			normalDecScaleStr,
			string("1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0"));
	makeScaleIdx(maxAccScaleStr, normalDecelerationScale);
	// speed limit add on
	string str;
	ParameterManager::Instance()->param(modelName, "speed_limit_add_on", str,
			string("-0.1911 -0.0708 -0.0082 0.0397 0.0810 0.1248 0.1661 0.2180 0.2745 0.3657"));
	makeScaleIdx(str, speedLimitAddon);

	// acc add on
	ParameterManager::Instance()->param(modelName, "Car_following_acceleration_add_on", str,
				string("-1.3564 -0.8547 -0.5562 -0.3178 -0.1036 0.1036 0.3178 0.5562 0.8547 1.3564"));
	sim_mob::Utils::convertStringToArray(str,accAddon);
	// max deceleration
	ParameterManager::Instance()->param(modelName, "Max_deceleration_car1",
			decelerationStr, string("16.0   14.5   13.0   11.0   10.0"));
	makeSpeedIndex(Vehicle::CAR, speedScalerStr, decelerationStr,
			maxDecelerationIndex, maxDecelerationUpperBound);
	ParameterManager::Instance()->param(modelName, "max_deceleration_scale",
			maxDecScaleStr, string("1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0"));
	makeScaleIdx(maxDecScaleStr, maxDecelerationScale);
	// acceleration grade factor
	ParameterManager::Instance()->param(modelName, "acceleration_grade_factor",
			accGradeFactor, 0.305);
	ParameterManager::Instance()->param(modelName, "tmp_all_grades", tmpGrade,
			0.0);
	// param for distanceToNormalStop()
	ParameterManager::Instance()->param(modelName, "min_speed", minSpeed, 0.1);
	ParameterManager::Instance()->param(modelName, "min_response_distance",
			minResponseDistance, 5.0);
	// param of calcSignalRate()
	ParameterManager::Instance()->param(modelName, "yellow_stop_headway",
			yellowStopHeadway, 1.0);
	ParameterManager::Instance()->param(modelName, "min_speed_yellow",
			minSpeedYellow, 2.2352);
	// param of carFollowingRate()
	ParameterManager::Instance()->param(modelName, "hbuffer_lower",
			hBufferLower, 0.8);
	string hBufferUpperStr;
	ParameterManager::Instance()->param(modelName, "hbuffer_Upper",
			hBufferUpperStr,
			string(
					"1.7498 2.2737 2.5871 2.8379 3.0633 3.2814 3.5068 3.7578 4.0718 4.5979"));
	makeScaleIdx(hBufferUpperStr, hBufferUpperScale);
	hBufferUpper = getBufferUppder();
	// Car following parameters
	string cfParamStr;
	ParameterManager::Instance()->param(modelName, "CF_parameters_1",
			cfParamStr,
			string("0.0400, 0.7220, 0.2420, 0.6820, 0.6000, 0.8250"));
	makeCFParam(cfParamStr, CF_parameters[0]);
	ParameterManager::Instance()->param(modelName, "CF_parameters_2",
			cfParamStr, string("-0.0418 0.0000 0.1510 0.6840 0.6800 0.8020"));
	makeCFParam(cfParamStr, CF_parameters[1]);
	//
	string targetGapAccParmStr;
	ParameterManager::Instance()->param(modelName, "target_gap_acc_parm",
			targetGapAccParmStr,
			string(
					"0.604, 0.385, 0.323, 0.0678, 0.217,0.583, -0.596, -0.219, 0.0832, -0.170, 1.478, 0.131, 0.300"));
	makeScaleIdx(targetGapAccParmStr, targetGapAccParm);
	//UPDATE STEP SIZE (REACTION TIME RELATED)
	string updateStepSizeStr;
	// dec
	ParameterManager::Instance()->param(modelName, "dec_update_step_size",
			updateStepSizeStr, string("0.5     0.0     0.5     0.5 0.5"));
	makeUpdateSizeParam(updateStepSizeStr, decUpdateStepSize);
	//speed factor
	ParameterManager::Instance()->param(modelName, "speed_factor", speedFactor,
			1.0);
	// acc
	ParameterManager::Instance()->param(modelName, "acc_update_step_size",
			updateStepSizeStr, string("1.0     0.0     1.0     1.0 0.5"));
	makeUpdateSizeParam(updateStepSizeStr, accUpdateStepSize);
	// uniform speed
	ParameterManager::Instance()->param(modelName,
			"uniform_speed_update_step_size", updateStepSizeStr,
			string("1.0     0.0     1.0     1.0 0.5"));
	makeUpdateSizeParam(updateStepSizeStr, uniformSpeedUpdateStepSize);
	// stopped vehicle
	ParameterManager::Instance()->param(modelName,
			"stopped_vehicle_update_step_size", updateStepSizeStr,
			string("0.5     0.0     0.5     0.5 0.5"));
	makeUpdateSizeParam(updateStepSizeStr, stoppedUpdateStepSize);
	boost::random_device seed_gen;
	long int r = seed_gen();
	updateSizeRm = boost::mt19937(r);
	calcUpdateStepSizes();

	// init step size , i=3 stopped vehicle
	p.nextStepSize = updateStepSize[3];
	nextPerceptionSize = perceptionSize[3];

	// visibility
	ParameterManager::Instance()->param(modelName, "visibility_distance",
			visibilityDistance, 10.0);

	// merge model
	ParameterManager::Instance()->param(modelName,
				"Merging_Model", str,
				string("10 20 8 0.2"));
	sim_mob::Utils::convertStringToArray(str,accAddon);
}
void sim_mob::MITSIM_CF_Model::makeCFParam(string& s, CarFollowParam& cfParam) {
	std::vector<std::string> arrayStr;
	vector<double> c;
	boost::trim(s);
	boost::split(arrayStr, s, boost::is_any_of(splitDelimiter),
			boost::token_compress_on);
	for (int i = 0; i < arrayStr.size(); ++i) {
		double res;
		try {
			res = boost::lexical_cast<double>(arrayStr[i].c_str());
		} catch (boost::bad_lexical_cast&) {
			std::string str = "can not covert <" + s + "> to double.";
			throw std::runtime_error(str);
		}
		c.push_back(res);
	}
	cfParam.alpha = c[0];
	cfParam.beta = c[1];
	cfParam.gama = c[2];
	cfParam.lambda = c[3];
	cfParam.rho = c[4];
	cfParam.stddev = c[5];
}
void sim_mob::MITSIM_CF_Model::makeUpdateSizeParam(string& s,
		UpdateStepSizeParam& sParam) {
	std::vector<std::string> arrayStr;
	vector<double> c;
	boost::trim(s);
	boost::split(arrayStr, s, boost::is_any_of(splitDelimiter),
			boost::token_compress_on);
	for (int i = 0; i < arrayStr.size(); ++i) {
		double res;
		try {
			res = boost::lexical_cast<double>(arrayStr[i].c_str());
		} catch (boost::bad_lexical_cast&) {
			std::string str = "can not covert <" + s + "> to double.";
			throw std::runtime_error(str);
		}
		c.push_back(res);
	}
	sParam.mean = c[0];
	sParam.stdev = c[1];
	sParam.lower = c[2];
	sParam.upper = c[3];
	sParam.percep = c[4];
}
void sim_mob::MITSIM_CF_Model::makeScaleIdx(string& s, vector<double>& c) {
	std::vector<std::string> arrayStr;
	boost::trim(s);
	boost::split(arrayStr, s, boost::is_any_of(splitDelimiter),
			boost::token_compress_on);
	for (int i = 0; i < arrayStr.size(); ++i) {
		double res;
		try {
			res = boost::lexical_cast<double>(arrayStr[i].c_str());
		} catch (boost::bad_lexical_cast&) {
			std::string str = "can not covert <" + s + "> to double.";
			throw std::runtime_error(str);
		}
		c.push_back(res);
	}
}
void sim_mob::MITSIM_CF_Model::makeSpeedIndex(VehicleBase::VehicleType vhType,
		string& speedScalerStr, string& cstr,
		map<VehicleBase::VehicleType, map<int, double> >& idx,
		int& upperBound) {
	std::cout << "makeSpeedIndex: vh type " << vhType << std::endl;
	// for example
	// speedScalerStr "5 20 20" ft/sec
	// maxAccStr      "10.00  7.90  5.60  4.00  4.00" ft/(s^2)
	std::vector<std::string> arrayStr;
	boost::trim(speedScalerStr);
	boost::split(arrayStr, speedScalerStr, boost::is_any_of(splitDelimiter),
			boost::token_compress_on);
	std::vector<double> speedScalerArrayDouble;
	for (int i = 0; i < arrayStr.size(); ++i) {
		double res;
		try {
			res = boost::lexical_cast<double>(arrayStr[i].c_str());
		} catch (boost::bad_lexical_cast&) {
			std::string str = "can not covert <" + speedScalerStr
					+ "> to double.";
			throw std::runtime_error(str);
		}
		speedScalerArrayDouble.push_back(res);
	}
	arrayStr.clear();
	//
	boost::algorithm::trim(cstr);
//	std::vector<std::string> maxAccArrayStr;
	boost::split(arrayStr, cstr, boost::is_any_of(splitDelimiter),
			boost::token_compress_on);
	std::vector<double> cArrayDouble;
	for (int i = 0; i < arrayStr.size(); ++i) {
		double res;
		try {
			res = boost::lexical_cast<double>(arrayStr[i].c_str());
		} catch (boost::bad_lexical_cast&) {
			std::string str = "can not covert <" + cstr + "> to double.";
			throw std::runtime_error(str);
		}
		cArrayDouble.push_back(res);
	}
	//
	upperBound = round(
			speedScalerArrayDouble[1] * (speedScalerArrayDouble[0] - 1));
	map<int, double> cIdx;
	for (int speed = 0; speed <= upperBound; ++speed) {
		double maxAcc;
		// Convert speed value to a table index.
		int j = speed / speedScalerArrayDouble[1];
		if (j >= (speedScalerArrayDouble[0] - 1)) {
			maxAcc = cArrayDouble[speedScalerArrayDouble[0] - 1];
		} else {
			maxAcc = cArrayDouble[j];
		}
		cIdx.insert(std::make_pair(speed, maxAcc));

//		std::cout<<"speed: "<<speed<<" max acc: "<<maxAcc<<std::endl;
	}

	idx[vhType] = cIdx;
}
double sim_mob::MITSIM_CF_Model::getMaxAcceleration(
		sim_mob::DriverUpdateParams& p, VehicleBase::VehicleType vhType) {
	if (!p.driver) {
		throw std::runtime_error("no driver");
	}

	// convert speed to int
	int speed = round(p.perceivedFwdVelocity / 100);
	if (speed < 0)
		speed = 0;
	if (speed > maxAccUpperBound)
		speed = maxAccUpperBound;

	double maxTableAcc = maxAccIndex[vhType][speed];

	// TODO: get random multiplier from data file and normal distribution

	double maxAcc = (maxTableAcc - tmpGrade * accGradeFactor)
			* getMaxAccScale();

	return maxAcc;
}
double sim_mob::MITSIM_CF_Model::getNormalDeceleration(
		sim_mob::DriverUpdateParams& p, VehicleBase::VehicleType vhType) {
	if (!p.driver) {
		throw std::runtime_error("no driver");
	}

	// convert speed to int
	int speed = round(p.perceivedFwdVelocity / 100);
	if (speed < 0)
		speed = 0;
	if (speed > normalDecelerationUpperBound)
		speed = normalDecelerationUpperBound;

	double normalDec = normalDecelerationIndex[vhType][speed];

	double dec = (normalDec - tmpGrade * accGradeFactor) * getNormalDecScale();

	return dec;
}
double sim_mob::MITSIM_CF_Model::getMaxDeceleration(
		sim_mob::DriverUpdateParams& p, VehicleBase::VehicleType vhType) {
	if (!p.driver) {
		throw std::runtime_error("no driver");
	}

	// convert speed to int
	int speed = round(p.perceivedFwdVelocity / 100);
	if (speed < 0)
		speed = 0;
	if (speed > maxDecelerationUpperBound)
		speed = maxDecelerationUpperBound;

	double maxDec = maxDecelerationIndex[vhType][speed];

	double dec = (maxDec - tmpGrade * accGradeFactor) * getMaxDecScale();

	return dec;
}
double sim_mob::MITSIM_CF_Model::getMaxAccScale() {
	// get random number (uniform distribution), as Random::urandom(int n) in MITSIM Random.cc
	int scaleNo = Utils::generateInt(1, maxAccScale.size() - 1);
	double res = Utils::generateFloat(maxAccScale[scaleNo-1],maxAccScale[scaleNo]);
	// return max acc scale,as maxAccScale() in MITSIM TS_Parameter.h
	return res;//maxAccScale[scaleNo];
}
double sim_mob::MITSIM_CF_Model::getNormalDecScale() {
	// get random number (uniform distribution), as Random::urandom(int n) in MITSIM Random.cc
	int scaleNo = Utils::generateInt(1, normalDecelerationScale.size() - 1);
	double res = Utils::generateFloat(normalDecelerationScale[scaleNo-1],normalDecelerationScale[scaleNo]);
	// return normal dec scale,as maxAccScale() in MITSIM TS_Parameter.h
	return res;//normalDecelerationScale[scaleNo];
}
double sim_mob::MITSIM_CF_Model::getMaxDecScale() {
	// get random number (uniform distribution), as Random::urandom(int n) in MITSIM Random.cc
	int scaleNo = Utils::generateInt(1, maxDecelerationScale.size() - 1);
	double res = Utils::generateFloat(normalDecelerationScale[scaleNo-1],normalDecelerationScale[scaleNo]);
	// return max dec scale,as maxAccScale() in MITSIM TS_Parameter.h
	return res;//maxDecelerationScale[scaleNo];
}
double sim_mob::MITSIM_CF_Model::getSpeedLimitAddon() {
	// get random number (uniform distribution), as Random::urandom(int n) in MITSIM Random.cc
	int scaleNo = Utils::generateInt(1, speedLimitAddon.size() - 1);
	double res = Utils::generateFloat(speedLimitAddon[scaleNo-1],speedLimitAddon[scaleNo]);
	return res;//speedLimitAddon[scaleNo];
}
double sim_mob::MITSIM_CF_Model::getAccAddon()
{
	int scaleNo = Utils::generateInt(1, accAddon.size() - 1);
	double res = Utils::generateFloat(accAddon[scaleNo-1],accAddon[scaleNo]);
	return res;//accAddon[scaleNo];
}
double sim_mob::MITSIM_CF_Model::getBufferUppder() {
	// get random number (uniform distribution), as Random::urandom(int n) in MITSIM Random.cc
	int scaleNo = Utils::generateInt(1, hBufferUpperScale.size() - 1);
	double res = Utils::generateFloat(hBufferUpperScale[scaleNo-1],hBufferUpperScale[scaleNo]);
	// return max acc scale,as maxAccScale() in MITSIM TS_Parameter.h
	return res;
}
double sim_mob::MITSIM_CF_Model::headwayBuffer() {
	return Utils::generateFloat(hBufferLower, hBufferUpper);
}

double sim_mob::MITSIM_CF_Model::makeAcceleratingDecision(DriverUpdateParams& p,
		double targetSpeed, double maxLaneSpeed) {
//	cftimer -= p.elapsedSeconds;
	/// if time cftimer >0 , return last calculated acc
	if (p.cftimer > sim_mob::Math::DOUBLE_EPSILON) {
		if(p.lastAcc > 10){
			int i = 0;
		}
		return p.lastAcc;
	}

//	// check if "performing lane change"
//	if(p.getStatus(STATUS_LC_CHANGING))
//	{
//		// return
//	}

	// VARIABLE || FUNCTION ||				REGIME
	calcStateBasedVariables(p);

	p.targetSpeed = calcDesiredSpeed(p);

	double acc = p.maxAcceleration; p.accSelect = "max";
	double aB = calcMergingRate(p);
	double aC = calcSignalRate(p); // near signal or incidents
	double aD = calcYieldingRate(p); // when yielding
	//double aE = waitExitLaneRate(p); //
	double aF = waitAllowedLaneRate(p);
//	double  aG = calcLaneDropRate(p);		// MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)
//	double aH1 = calcAdjacentRate(p); // to reach adjacent gap
//	double aH2 = calcBackwardRate(p); // to reach backward gap
//	double aH3 = calcForwardRate(p); // to reach forward gap
	// The target gap acceleration should be based on the target gap status and not on the min
	// MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)
	double aH = p.maxAcceleration;
	if(!p.getStatus(STATUS_LC_CHANGING)) // not in middle of performing lane change
	{
	 if (p.getStatus(STATUS_ADJACENT)) {
	  aH = calcAdjacentRate(p);	// to reach adjacent gap
	 }
	 else if (p.getStatus(STATUS_BACKWARD)) {
	  aH = calcBackwardRate(p);	// to reach backward gap
	 }
	 else if (p.getStatus(STATUS_FORWARD)) {
	  aH = calcForwardRate(p);		// to reach forward gap
	 } else {
		  aH = desiredSpeedRate(p);
	 }
	}


	// if (intersection){
	// double aI = approachInter(p); // when approaching intersection to achieve the turn speed
	// if(acc > aI) acc = aI;
	// }
	// FUNCTION approachInter MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)
//	double aZ1 = carFollowingRate(p, p.nvFwd);
//	double aZ2 = carFollowingRate(p, p.nvFwdNextLink);
	double aZ = calcCarFollowingRate(p);

	// Make decision
	// Use the smallest
	//	if(acc > aB) acc = aB;

	if (acc > aD)
	{
		acc = aD;
		p.accSelect = "aD";
	}
	//if(acc > aF) acc = aF;
	if (acc > aH) {
		acc = aH;
		p.accSelect = "aH";
	}
//	if (acc > aH1)
//		acc = aH1;
//	if (acc > aH2)
//		acc = aH2;
//	if (acc > aH3)
//		acc = aH3;
	//if(acc > aG) acc = aG;
	if (acc > aC) {
		acc = aC;
		p.accSelect = "aC";
	}
//	if (acc > aE)
//		acc = aE;
	if (acc > aZ) {
		acc = aZ;
		p.accSelect = "aZ";
	}
//	if (acc > aZ1)
//		acc = aZ1;
//	if (acc > aZ2)
//		acc = aZ2;

	// SEVERAL CONDITONS MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)

	p.lastAcc = acc;

	// if brake ,alerm follower
	if (acc < -ACC_EPSILON) {

		// I am braking, alert the vehicle behind if it is close

	//	TS_Vehicle *back = findFrontBumperFollower(lane_);
		if (p.nvBack.exists() )
		{
			Driver* bvd = const_cast<Driver*>(p.nvBack.driver);
			DriverUpdateParams& bvp = bvd->getParams();
			if( p.nvBack.distance < visibility() &&
					!(bvd->isBus() && bvp.getStatus(STATUS_STOPPED))) {
						  float alert = CF_CRITICAL_TIMER_RATIO * updateStepSize[0];
						  bvp.cftimer = std::min<double>(alert,bvp.cftimer);
					//	  back->cfTimer_ = Min(alert, back->cfTimer_);
						}
		}
	}


	// if in emergency regime , reduce cftimer
	p.cftimer = calcNextStepSize(p);
	if (p.getStatus(STATUS_REGIME_EMERGENCY) ) {
		p.cftimer = p.getNextStepSize()* CF_CRITICAL_TIMER_RATIO;
	}
	else {
		p.cftimer = p.getNextStepSize();
	}

	if(acc > 10){
		int i = 0;
	}
	return acc;
}

/*
 *--------------------------------------------------------------------
 * Calculate acceleration rate by car-following constraint. This
 * function may also be used in the lane changing algorithm to find
 * the potential acceleration rate in neighbor lanes.
 *
 * CAUTION: The two vehicles concerned in this function may not
 * necessarily be in the same lane or even the same segment.
 *
 * A modified GM model is used in this implementation.
 *--------------------------------------------------------------------
 */
double sim_mob::MITSIM_CF_Model::carFollowingRate(DriverUpdateParams& p,
		NearestVehicle& nv) {
	// unset status
	p.unsetStatus(STATUS_REGIME_EMERGENCY);

	p.space = p.perceivedDistToFwdCar / 100;

	double res = 0;
	//If we have no space left to move, immediately cut off acceleration.
//	if ( p.space < 2.0 && p.isAlreadyStart )
//		return maxDeceleration;
	if (p.space < 2.0 && p.isAlreadyStart && p.isBeforIntersecton
			&& p.perceivedFwdVelocityOfFwdCar / 100 < 1.0) {
		return p.maxDeceleration * 4.0;
	}
	if (p.space > 0) {
		if (!nv.exists()) {
			return accOfFreeFlowing(p, p.targetSpeed, p.maxLaneSpeed);
		}
		// when nv is left/right vh , can not use perceivedxxx!
//		p.v_lead = p.perceivedFwdVelocityOfFwdCar/100;
//		p.a_lead = p.perceivedAccelerationOfFwdCar/100;

		p.v_lead = nv.driver->fwdVelocity / 100;
		p.a_lead = nv.driver->fwdAccel / 100;

//		double dt	=	p.elapsedSeconds;
		double dt = p.nextStepSize;
		double headway = CalcHeadway(p.space, p.perceivedFwdVelocity / 100,
				p.elapsedSeconds, p.maxAcceleration);
//		std::cout<<"carFollowingRate: headway1: "<<headway<<std::endl;

		//Emergency deceleration overrides the perceived distance; check for it.
		{
//			double emergSpace = p.perceivedDistToFwdCar/100;
			double emergSpace = nv.distance / 100;
			double emergHeadway = CalcHeadway(emergSpace,
					p.perceivedFwdVelocity / 100, p.elapsedSeconds,
					p.maxAcceleration);
			if (emergHeadway < hBufferLower) {
				//We need to brake. Override.
				p.space = emergSpace;
				headway = emergHeadway;
			}
		}

		p.space_star = p.space + p.v_lead * dt + 0.5 * p.a_lead * dt * dt;
//		std::cout<<"carFollowingRate: headway2: "<<headway<<std::endl;
		if (headway < hBufferLower) {
			res = accOfEmergencyDecelerating(p);
			p.setStatus(STATUS_REGIME_EMERGENCY);
//			std::cout<<"carFollowingRate: EmergencyDecelerating: "<<res<<std::endl;
		}
		hBufferUpper = getBufferUppder();
		if (headway > hBufferUpper) {
			res = accOfMixOfCFandFF(p, p.targetSpeed, p.maxLaneSpeed);
		}
		if (headway <= hBufferUpper && headway >= hBufferLower) {
			res = accOfCarFollowing(p);
		}

//		if(p.isWaiting && p.dis2stop<5000 && res > 0)
//		{
//			res=res*(5000.0-p.dis2stop)/5000.0;
//			if(p.dis2stop<2000)
//			{
//				res=0;
//			}
//		}
	}
	return res;
}
double sim_mob::MITSIM_CF_Model::calcCarFollowingRate(DriverUpdateParams& p)
{
//	status_ &= ~(STATUS_REGIME);
 // need unset STATUS_REGIME?
	double acc;
	if(isInMergingArea(p))
	{

	}
	// as isInMergingArea() not function now
//	else /
	{
		if(p.now.frame()>244 && p.now.frame()< 250 && p.driver->getParent()->GetId()==1){
			int ii =0;
		}
		double aZ1 = carFollowingRate(p, p.nvFwd);
		double aZ2 = carFollowingRate(p, p.nvFwdNextLink);
		if(aZ1<aZ2) {
			acc = aZ1;
		}
		else {
			acc = aZ2;
		}
	}

	return acc;

}
int sim_mob::MITSIM_CF_Model::isInMergingArea(DriverUpdateParams& p)
{
	//TODO: uninode lane connector has issue
	// check buildForwardLanesFromAlignedLanes(), each lane connector to next segment's left,center,right lane
	// it is impossible to find merging area, which has lane merge area

	return 0;
// not use FLAG_MERGING as in MITSIM, it maybe used to fix certain bug

	// check current lane connect to two lane and vehicle is in the lower part of the merging area

//	DriverMovement *driverMvt = (DriverMovement*) p.driver->Movement();
//	if(driverMvt->fwdDriverMovement.getDisToCurrSegEndM() < dnMergingArea())
//	{
//		const UniNode* currEndNode = dynamic_cast<const UniNode*> (driverMvt->fwdDriverMovement.getCurrSegment()->getEnd());
//		if(currEndNode)
//		{
//
//		}
//	}



}
double sim_mob::MITSIM_CF_Model::calcMergingRate(
		sim_mob::DriverUpdateParams& p) {
	double acc = p.maxAcceleration;
	// TS_Vehicles from freeways and on ramps have different
	// priority.  Seperate procedures are applied. (MITSIM TS_CFModels.cc)
	//  if (lane_->linkType() == LINK_TYPE_FREEWAY) {
	DriverMovement *driverMvt = (DriverMovement*) p.driver->Movement();
	if (driverMvt->fwdDriverMovement.getCurrSegment()->type
			== sim_mob::LINK_TYPE_FREEWAY) // current on freeway
			{
		if (p.nvLeadFreeway.exists()) // has lead vh on next link
		{
			double headway = headwayBuffer(); //TODO: headway is correct?
			if (p.nvLeadFreeway.distance / 100.0 < headway) {
				// MITSIM TS_CFModels.cc
				// acc = first->accRate_ + brakeToTargetSpeed(distance_, first->currentSpeed_);
				acc = p.nvLeadFreeway.driver->fwdAccel
						+ brakeToTargetSpeed(p,
								p.nvLeadFreeway.distance / 100.0,
								p.nvLeadFreeway.driver->fwdVelocity);
			}
		}
	} else if (driverMvt->fwdDriverMovement.getCurrSegment()->type
			== sim_mob::LINK_TYPE_RAMP) // on ramp
			{
		if (p.nvLagFreeway.exists()) // has lag vh on freeway
		{
			// MITSIM TS_CFModels.cc
			//if (second->distance_ < distance_ ||
			//       second->currentSpeed_ > currentSpeed_ + maxAcceleration_ ||
			//     !theRandomizer->brandom(theParameter->aggresiveRampMergeProb()))

			if (!isGapAcceptable(p, p.nvLagFreeway)) {

				// The gap is not acceptable. Prepare to stop.

				return brakeToStop(p, p.dis2stop);
			}
		} else {
			if (p.nvLeadFreeway.exists()) // find the lead vh in the next link
			{
				// lead exists, brake to target speed
				acc = brakeToTargetSpeed(p, p.nvLeadFreeway.distance / 100.0,
						p.nvLeadFreeway.driver->fwdVelocity);
			}
		}
	}

	return acc;
}
bool sim_mob::MITSIM_CF_Model::isGapAcceptable(sim_mob::DriverUpdateParams& p,
		NearestVehicle& vh) {
	float accn = p.maxAcceleration;
	float speedm = vh.driver->fwdVelocity / 100.0; //coming->currentSpeed_;
	// get object vh's max acceleration
//	DriverMovement *driverMvt = (DriverMovement*)vh.driver->Movement();
	Driver* d = const_cast<Driver*>(vh.driver);
	DriverUpdateParams& dp = d->getParams();
	float accm = dp.maxAcceleration; //coming->maxAcceleration();
	float speedn;
//	double dt = p.elapsedSeconds;
	double dt = p.nextStepSize;

	double distance = p.dis2stop; // distance to end of the link
	double currentSpeed = p.driver->fwdVelocity / 100.0; // subject vh current speed m/s

	if (distance > sim_mob::Math::DOUBLE_EPSILON) {
		// maximum speed can be reached at the end of the lane.

		speedn = currentSpeed * currentSpeed + 2.0 * accn * distance;
		speedn = (speedn > sim_mob::Math::DOUBLE_EPSILON) ? sqrt(speedn) : 0.0;

		// shortest time required to arrive at the end of the lane

		dt = (speedn - currentSpeed) / accn;
	} else {
		speedn = currentSpeed;
		dt = 0;
	}

	// Max distance traveled by vehicle m in time dt

	float dism = speedm * dt + 0.5 * accm * dt * dt;
	float gap_mn = vh.distance / 100.0;

	// Speed at the pridicted position

	speedm += accm * dt;
	float sd = (speedm - speedn) * headwayBuffer();
	float threshold = (sd > 0.0) ? sd : 0.0;

	// check if the gap is acceptable

	if ((gap_mn > threshold))
		return true;
	else
		return 0;
}
double sim_mob::MITSIM_CF_Model::calcSignalRate(DriverUpdateParams& p) {
	double minacc = p.maxAcceleration;
//	double yellowStopHeadway = 1; //1 second
//	double minSpeedYellow = 2.2352;//5 mph = 2.2352 m / s

	sim_mob::TrafficColor color;

#if 0
	Signal::TrafficColor color;
#endif
	double distanceToTrafficSignal;
	distanceToTrafficSignal = p.perceivedDistToTrafficSignal;
	color = p.perceivedTrafficColor;
	double dis = p.perceivedDistToFwdCar;
	if (distanceToTrafficSignal < 5100) {
		double dis = distanceToTrafficSignal / 100;

#if 0
		if(p.perceivedTrafficColor == sim_mob::Red)
		{
			double a = brakeToStop(p, dis);
			if(a < minacc)
			minacc = a;
		}
		else if(p.perceivedTrafficColor == sim_mob::Amber)
		{
			double maxSpeed = (speed>minSpeedYellow)?speed:minSpeedYellow;
			if(dis/maxSpeed > yellowStopHeadway)
			{
				double a = brakeToStop(p, dis);
				if(a < minacc)
				minacc = a;
			}
		}
		else if(p.perceivedTrafficColor == sim_mob::Green)
		{
			minacc = maxAcceleration;
		}
#else
		if (color == sim_mob::Red)
#if 0
				if(color == Signal::Red)
#endif
				{
			double a = brakeToStop(p, dis);
			if (a < minacc)
				minacc = a;
		} else if (color == sim_mob::Amber)
#if 0
				else if(color == Signal::Amber)
#endif
				{
			double maxSpeed =
					(p.perceivedFwdVelocity / 100 > minSpeedYellow) ?
							p.perceivedFwdVelocity / 100 : minSpeedYellow;
			if (dis / maxSpeed > yellowStopHeadway) {
				double a = brakeToStop(p, dis);
				if (a < minacc)
					minacc = a;
			}
		} else if (color == sim_mob::Green)
#if 0
				else if(color == Signal::Green)
#endif
				{
			minacc = p.maxAcceleration;
		}

#endif

	}
	return minacc;
}

double sim_mob::MITSIM_CF_Model::calcYieldingRate(DriverUpdateParams& p) {
	float acc;

	if (p.flag(FLAG_YIELDING)) {
		// Make sure a vehicle will not yield infinitely.
		uint32_t dt_sec = (p.now.ms() - p.yieldTime.ms()) / 1000;
		if (dt_sec > p.lcMaxNosingTime) {
			p.driver->yieldVehicle = NULL;
			p.unsetFlag(FLAG_YIELDING);
			return p.maxAcceleration;
		} //end of lcMaxNosingTime

		// This vehicle is yielding to another vehicle

		bool rightFwdVhFlag = false;
		if (p.nvRightFwd.exists()) {
			Driver* d = const_cast<Driver*>(p.nvRightFwd.driver);
			DriverUpdateParams& p = d->getParams();
			if (p.flag(FLAG_NOSING_LEFT)) {
				rightFwdVhFlag = true;
			}
		}

		bool leftFwdVhFlag = false;
		if (p.nvLeftFwd.exists()) {
			Driver* d = const_cast<Driver*>(p.nvLeftFwd.driver);
			DriverUpdateParams& p = d->getParams();
			if (p.flag(FLAG_NOSING_LEFT)) {
				leftFwdVhFlag = true;
			}
		}

		if (p.flag(FLAG_YIELDING_RIGHT)) {
			if ((p.rightLane) && // right side has lane
					(p.nvRightFwd.exists()) && // right lane has fwd vh
					p.nvRightFwd.driver == p.driver->yieldVehicle && // the right fwd vh is nosing
					rightFwdVhFlag // right fwd vh nosing
					) {
				acc = carFollowingRate(p, p.nvRightFwd);
				if (acc < p.normalDeceleration) {
					acc = p.normalDeceleration;
				} else if (acc > 0) {
					acc = 0.0;
				}
				return acc;
			}
		} else if (p.flag(FLAG_YIELDING_LEFT)) {
			if ((p.leftLane) && // left side has lane
					(p.nvLeftFwd.exists()) && // left lane has fwd vh
					p.nvLeftFwd.driver == p.driver->yieldVehicle && // the left fwd vh is nosing
					leftFwdVhFlag) {
				acc = carFollowingRate(p, p.nvLeftFwd);
				if (acc < p.normalDeceleration) {
					acc = p.normalDeceleration;
				} else if (acc > 0) {
					acc = 0.0;
				}
				return acc;
			}
		} // end of else

		p.driver->yieldVehicle = NULL;
		p.unsetFlag(FLAG_YIELDING);

		return p.maxAcceleration;

	} //end if flag(FLAG_YIELDING)
	else if (p.flag(FLAG_NOSING)) {

		// This vehicle is nosing
		bool rightBackVhFlag = false;
		if (p.nvRightFwd.exists()) {
			Driver* d = const_cast<Driver*>(p.nvRightBack.driver);
			DriverUpdateParams& p = d->getParams();
			if (p.flag(FLAG_YIELDING_LEFT)) {
				rightBackVhFlag = true;
			}
		}
		bool leftBackVhFlag = false;
		if (p.nvRightFwd.exists()) {
			Driver* d = const_cast<Driver*>(p.nvLeftBack.driver);
			DriverUpdateParams& p = d->getParams();
			if (p.flag(FLAG_YIELDING_LEFT)) {
				leftBackVhFlag = true;
			}
		}

		if (p.flag(FLAG_NOSING_RIGHT)) {
			if ((p.rightLane) && // has right lane
					(p.nvRightBack.exists()) && // has right back vh
					rightBackVhFlag) // right back vh yielding left
					{
				acc = calcCreateGapRate(p, p.nvRightFwd,
						p.lcMinGap(2) + Math::DOUBLE_EPSILON); //other->vehicleAhead(),theParameter->lcMinGap(2) + DIS_EPSILON);
				return std::max<double>(p.maxDeceleration, acc);
			}
		} else if (p.flag(FLAG_NOSING_LEFT)) {
			if ((p.leftLane) && // has left lane
					(p.nvLeftBack.exists()) && leftBackVhFlag) {
				acc = calcCreateGapRate(p, p.nvLeftFwd,
						p.lcMinGap(2) + Math::DOUBLE_EPSILON);
				return std::max<double>(p.maxDeceleration, acc);
			}
		}

		if (p.getStatus(STATUS_CHANGING) && p.flag(FLAG_LC_FAILED_LEAD)) {
			return p.normalDeceleration;
		} else {
			return p.maxAcceleration;
		}

	} //end if flag(FLAG_NOSING_RIGHT)
	else {

		// Currently this vehicle is neither yielding, nor nosing.

		return p.maxAcceleration;
	}
}
double sim_mob::MITSIM_CF_Model::calcCreateGapRate(DriverUpdateParams& p,
		NearestVehicle& vh, float gap) {
	// No vehicle ahead, this constraint does not apply

	if (!vh.exists())
		return p.maxAcceleration;

	// freedom left

	float dx = vh.distance - gap; //gapDistance(front) - gap;
	float dv = p.currSpeed - vh.driver->fwdVelocity / 100.0;

	float dt = p.nextStepSize;
	if (dt <= 0.0)
		return p.maxAcceleration;
	return p.driver->fwdAccel + 2.0 * (dx - dv * dt) / (dt * dt);
}
double sim_mob::MITSIM_CF_Model::waitExitLaneRate(DriverUpdateParams& p) {
//	double dx = p.dis2stop- 5;
	// dis2stop is distance to fwd vh or distance to end node

	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
	double dx = driverMvt->fwdDriverMovement.getDistToLinkEndM() - 5.0;

	if(!p.getStatus(STATUS_CURRENT_OK) && dx < p.distanceToNormalStop)
	{
		return brakeToStop(p,dx);
	}
	else
	{
		return p.maxAcceleration;
	}

//	double dx = p.perceivedDistToFwdCar / 100 - 5;
//	if (p.turningDirection == LCS_SAME || dx > p.distanceToNormalStop)
//		return p.maxAcceleration;
//	else
//		return brakeToStop(p, dx);
}
double sim_mob::MITSIM_CF_Model::waitAllowedLaneRate(
		sim_mob::DriverUpdateParams& p) {
	if (p.dis2stop < p.distanceToNormalStop && !p.isTargetLane) {
		if (p.flag(FLAG_NOSING)) {
			return p.maxAcceleration;
		}
		double acc = brakeToStop(p, p.distanceToNormalStop);
		return acc;
	}
	return p.maxAcceleration;
}
double sim_mob::MITSIM_CF_Model::calcDesiredSpeed(
		sim_mob::DriverUpdateParams& p) {
	double signedSpeed;
	if (p.speedOnSign) {
		signedSpeed = p.speedOnSign;
	} else {
		signedSpeed = p.maxLaneSpeed;
	}

	float desired = speedFactor * signedSpeed;
	desired = desired * (1 + getSpeedLimitAddon());

	double desiredSpeed = std::min<double>(desired, p.maxLaneSpeed);
	return desiredSpeed;
}
double sim_mob::MITSIM_CF_Model::calcForwardRate(DriverUpdateParams& p) {
//	TS_Lane *plane;

//	  if (status(STATUS_LEFT)) {
//		plane = lane_->left();
//	  } else if (status(STATUS_RIGHT)) {
//		plane = lane_->right();
//	  } else {
//		return MAX_ACCELERATION;	       	// No request for lane change
//	  }
//
//	  float *a = theParameter->targetGapParams();
	const NearestVehicle * av = NULL; // side leader vh

	if (p.getStatus(STATUS_LEFT)) {
	  av = &p.nvLeftFwd;
	} else if (p.getStatus(STATUS_RIGHT)) {
	  av = &p.nvRightFwd;
	} else {
	//		return MAX_ACCELERATION;			// No request for lane change
	  return p.maxAcceleration;
	}

//	 float *a = theParameter->targetGapParams();
	std::vector<double> a = targetGapAccParm;

//	    TS_Vehicle* av = findFrontBumperLeader(plane);
	double dis;
	double dv;
	if(av->exists())
	{
		dis = av->distance + a[0];
		Driver *avDriver = const_cast<Driver*>(av->driver);
		dv = avDriver->getFwdVelocityM() - p.driver->getFwdVelocityM();
	}
	else
	{
		dis = av->distance + a[0];
		dv = 0 - p.driver->getFwdVelocityM();
	}

	  float acc = a[1] * pow(dis, a[2]);

	  if (dv > 0) {
	    acc *= pow(dv, a[3]);
	  } else if (dv < 0) {
	    acc *= pow (-dv, a[4]);
	  }
//	  acc += theParameter->cfAccAddOn(driverGroup.cfAccAddOn) * a[5] /0.824 ;
	  acc += getAccAddon() * a[5] /0.824 ;
	  return acc;
//	/*
//	 if(p.turningDirection == LCS_SAME)
//	 return maxAcceleration;
//	 NearestVehicle& nv = (p.turningDirection == LCS_LEFT)?p.nvLeftFwd:p.nvRightFwd;
//	 */
//
//	if (p.targetGap != TG_Left_Fwd || p.targetGap != TG_Right_Fwd)
//		return p.maxAcceleration;
//	NearestVehicle& nv =
//			(p.targetGap == TG_Left_Fwd) ? p.nvLeftFwd : p.nvRightFwd;
//
//	if (!nv.exists())
//		return p.maxAcceleration;
//	double dis = nv.distance / 100 + targetGapAccParm[0];
//	double dv = nv.driver->fwdVelocity.get() / 100
//			- p.perceivedFwdVelocity / 100;
//	double acc = targetGapAccParm[1] * pow(dis, targetGapAccParm[2]);
//
//	if (dv > 0) {
//		acc *= pow(dv, targetGapAccParm[3]);
//	} else if (dv < 0) {
//		acc *= pow(-dv, targetGapAccParm[4]);
//	}
//	acc += targetGapAccParm[5] / 0.824;
//	return acc;
}

double sim_mob::MITSIM_CF_Model::calcBackwardRate(DriverUpdateParams& p) {
//	TS_Lane *plane;

//	if (status(STATUS_LEFT)) {
//	plane = lane_->left();
//	} else if (status(STATUS_RIGHT)) {
//	plane = lane_->right();
//	} else {
//	return MAX_ACCELERATION;	      // No request for lane change
//	}

	const NearestVehicle * bv = NULL; // side follower vh

	if (p.getStatus(STATUS_LEFT)) {
	  bv = &p.nvLeftBack;
	} else if (p.getStatus(STATUS_RIGHT)) {
	  bv = &p.nvRightBack;
	} else {
	//		return MAX_ACCELERATION;			// No request for lane change
	  return p.maxAcceleration;
	}
//	float *a = theParameter->targetGapParams();
	std::vector<double> a = targetGapAccParm;

	//	 TS_Vehicle* bv = findFrontBumperFollower(plane);
	double dis;
	double dv;
	 if (bv->exists()) {
		 // how come get here?
//		 return p.maxAcceleration;
		 Driver *bvDriver = const_cast<Driver*>(bv->driver);

		 //	float dis = av->gapDistance(this) + length() + a[0];
		 	dis = bv->distance + a[0];
		 //	float dv = av->currentSpeed() - currentSpeed();
		 	dv = bvDriver->getFwdVelocityM() - p.driver->getFwdVelocityM();
	 }
	 else
	 {
		 dis = bv->distance + a[0];
		 dv =  0 - p.driver->getFwdVelocityM();
	 }

	float acc = a[6] * pow(dis, a[7]);

	if (dv > 0) {
	acc *= pow(dv, a[8]);
	} else if (dv < 0) {
	acc *= pow (-dv, a[9]);
	}
//	acc += theParameter->cfAccAddOn(driverGroup.cfAccAddOn) *  a[10] / 0.824 ;
	acc += getAccAddon() *  a[10] / 0.824 ;
	return acc;
//	/*
//	 if(p.turningDirection == LCS_SAME)
//	 return maxAcceleration;
//	 //NearestVehicle& nv = (p.turningDirection == LCS_LEFT)?p.nvLeftFwd:p.nvRightFwd;
//	 NearestVehicle& nv = (p.turningDirection == LCS_LEFT)?p.nvLeftBack:p.nvRightBack;//change a mistake!!!
//	 */
//
//	if (p.targetGap != TG_Left_Back || p.targetGap != TG_Right_Back)
//		return p.maxAcceleration;
//	NearestVehicle& nv =
//			(p.targetGap == TG_Left_Back) ? p.nvLeftBack : p.nvRightBack;
//
//	if (!nv.exists())
//		return p.maxAcceleration;
//
//	double dis = nv.distance / 100 + targetGapAccParm[0];
//	double dv = nv.driver->fwdVelocity.get() / 100
//			- p.perceivedFwdVelocity / 100;
//
//	double acc = targetGapAccParm[6] * pow(dis, targetGapAccParm[7]);
//
//	if (dv > 0) {
//		acc *= pow(dv, targetGapAccParm[8]);
//	} else if (dv < 0) {
//		acc *= pow(-dv, targetGapAccParm[9]);
//	}
//	acc += targetGapAccParm[10] / 0.824;
//	return acc;
}

double sim_mob::MITSIM_CF_Model::calcAdjacentRate(DriverUpdateParams& p) {

	const NearestVehicle * av = NULL; // side leader vh
	const NearestVehicle * bv = NULL; // side follower vh

	  if (p.getStatus(STATUS_LEFT)) {
		  av = &p.nvLeftFwd;
		  bv = &p.nvLeftBack;
	  } else if (p.getStatus(STATUS_RIGHT)) {
		  av = &p.nvRightFwd;
		  bv = &p.nvRightBack;
	  } else {
//		return MAX_ACCELERATION;			// No request for lane change
		  return p.maxAcceleration;
	  }

//	 float *a = theParameter->targetGapParams();
	  std::vector<double> a = targetGapAccParm;

//	 TS_Vehicle* av = findFrontBumperLeader(plane);
	 if (!av->exists()) return p.maxAcceleration;

//	 TS_Vehicle* bv = findFrontBumperFollower(plane);
	 if (!bv->exists()) return p.maxAcceleration;

	 Driver *avDriver = const_cast<Driver*>(av->driver);
	 Driver *bvDriver = const_cast<Driver*>(bv->driver);

//	 float gap = bv->gapDistance(av);
//	 float position = bv->gapDistance(this)+ length();
//	 float acc = a[11] * (a[0] * gap - position);

	 float gap = bvDriver->gapDistance(avDriver);
	 float position = bvDriver->gapDistance(p.driver)+ p.driver->getVehicleLengthM();
	 float acc = a[11] * (a[0] * gap - position);

//	  acc += theParameter->cfAccAddOn(driverGroup.cfAccAddOn) * a[12] / 0.824 ;
	 acc += getAccAddon() * a[12] / 0.824 ;
	 return acc;
//	if (p.nextLaneIndex == p.currLaneIndex)
//		return p.maxAcceleration;
//	NearestVehicle& av =
//			(p.nextLaneIndex > p.currLaneIndex) ? p.nvLeftFwd : p.nvRightFwd;
//	NearestVehicle& bv =
//			(p.nextLaneIndex > p.currLaneIndex) ? p.nvLeftBack : p.nvRightBack;
//	if (!av.exists())
//		return p.maxAcceleration;
//	if (!bv.exists())
//		return p.normalDeceleration;
//	double gap = bv.distance / 100 + av.distance / 100;
//	double position = bv.distance / 100;
//	double acc = targetGapAccParm[11] * (targetGapAccParm[0] * gap - position);
//
//	acc += targetGapAccParm[12] / 0.824;
//	return acc;
}
/*
 *-------------------------------------------------------------------
 * This function returns the acceleration rate required to
 * accelerate / decelerate from current speed to a full
 * stop within a given distance.
 *-------------------------------------------------------------------
 */
double sim_mob::MITSIM_CF_Model::brakeToStop(DriverUpdateParams& p,
		double dis) {
//	double DIS_EPSILON =	0.001;
	if (dis > sim_mob::Math::DOUBLE_EPSILON) {
		double u2 = (p.perceivedFwdVelocity / 100)
				* (p.perceivedFwdVelocity / 100);
		double acc = -u2 / dis * 0.5;
		if (acc <= p.normalDeceleration)
			return acc;
//		double dt = p.elapsedSeconds;
		double dt = p.nextStepSize;
		double vt = p.perceivedFwdVelocity / 100 * dt;
		double a = dt * dt;
		double b = 2.0 * vt - p.normalDeceleration * a;
		double c = u2 + 2.0 * p.normalDeceleration * (dis - vt);
		double d = b * b - 4.0 * a * c;

		if (d < 0 || a <= 0.0)
			return acc;

		return (sqrt(d) - b) / a * 0.5;
	} else {

//		double dt = p.elapsedSeconds;
		double dt = p.nextStepSize;
		return (dt > 0.0) ?
				-(p.perceivedFwdVelocity / 100) / dt : p.maxDeceleration;
	}
}

/*
 *-------------------------------------------------------------------
 * Returns the acceleration rate constrained by the desired speed.
 * CLA 18/06/2014
 *-------------------------------------------------------------------
 */
double sim_mob::MITSIM_CF_Model::desiredSpeedRate(DriverUpdateParams& p)
{
  float maxspd = p.targetSpeed;
  double epsilon_v = sim_mob::Math::DOUBLE_EPSILON;
  if (p.perceivedFwdVelocity /100 < maxspd - epsilon_v) {
	// Use maximum acceleration
	return p.maxAcceleration;
  } else if (p.perceivedFwdVelocity /100 > maxspd + epsilon_v) {
	// Decelerate
	return p.normalDeceleration;
  } else {
	// Keep current speed.
	return 0.0;
  }
}

/*
 *-------------------------------------------------------------------
 * This function returns the acceleration rate required to
 * accelerate / decelerate from current speed to a target
 * speed within a given distance.
 *-------------------------------------------------------------------
 */
double sim_mob::MITSIM_CF_Model::brakeToTargetSpeed(DriverUpdateParams& p,
		double s, double v) {
//	double v 			=	p.perceivedFwdVelocity/100;
//	double dt			=	p.elapsedSeconds;
	double dt = p.nextStepSize;

//	//NOTE: This is the only use of epsilon(), so I just copied the value directly.
//	//      See LC_Model for how to declare a private temporary variable. ~Seth
//	if(p.space_star > sim_mob::Math::DOUBLE_EPSILON) {
//		return  ((p.v_lead + p.a_lead * dt ) * ( p.v_lead + p.a_lead * dt) - v * v) / 2 / p.space_star;
//	} else if ( dt <= 0 ) {
//		return maxAcceleration;
//	} else {
//		return ( p.v_lead + p.a_lead * dt - v ) / dt;
//	}

	double currentSpeed_ = p.perceivedFwdVelocity / 100;
	if (s > sim_mob::Math::DOUBLE_EPSILON) {
		float v2 = v * v;
		float u2 = currentSpeed_ * currentSpeed_;
		float acc = (v2 - u2) / s * 0.5;

		return acc;
	} else {
		//	float dt = nextStepSize;
//		if (dt <= 0.0) return maxAcceleration ;
		return (v - currentSpeed_) / dt;
	}
}

double sim_mob::MITSIM_CF_Model::accOfEmergencyDecelerating(
		DriverUpdateParams& p) {
	double v = p.perceivedFwdVelocity / 100;
	double dv = v - p.v_lead;
	double epsilon_v = sim_mob::Math::DOUBLE_EPSILON;
	double aNormalDec = p.normalDeceleration;

	double a;
	if (dv < epsilon_v) {
		a = p.a_lead + 0.25 * aNormalDec;
	} else if (p.space > 0.01) {
		a = p.a_lead - dv * dv / 2 / p.space;
	} else {
//		double dt	=	p.elapsedSeconds;
		double dt = p.nextStepSize;
		//p.space_star	=	p.space + p.v_lead * dt + 0.5 * p.a_lead * dt * dt;
		double s = p.space_star;
		double v = p.v_lead + p.a_lead * dt;
		a = brakeToTargetSpeed(p, s, v);
	}
	return min(p.normalDeceleration, a);
}

double sim_mob::MITSIM_CF_Model::accOfCarFollowing(DriverUpdateParams& p) {
	const double density = 0; //represent the density of vehicles in front of the subject vehicle
							  //now we ignore it, assuming that it is 0.
	double v = p.perceivedFwdVelocity / 100;
	int i = (v > p.v_lead) ? 1 : 0;
	double dv = (v > p.v_lead) ? (v - p.v_lead) : (p.v_lead - v);

	double res = CF_parameters[i].alpha * pow(v, CF_parameters[i].beta)
			/ pow(p.nvFwd.distance / 100, CF_parameters[i].gama);
	res *= pow(dv, CF_parameters[i].lambda)
			* pow(density, CF_parameters[i].rho);
//	res += feet2Unit(nRandom(p.gen, 0, CF_parameters[i].stddev));
	res += feet2Unit(Utils::nRandom(0, CF_parameters[i].stddev));

	return res;
}

double sim_mob::MITSIM_CF_Model::accOfFreeFlowing(DriverUpdateParams& p,
		double targetSpeed, double maxLaneSpeed) {
	double vn = p.perceivedFwdVelocity / 100;
	if (vn < targetSpeed) {
		return (vn < maxLaneSpeed) ? p.maxAcceleration : 0;
	} else if (vn > targetSpeed) {
		return 0;
	}

	//If equal:
	return (vn < maxLaneSpeed) ? p.maxAcceleration : 0;
}

double sim_mob::MITSIM_CF_Model::accOfMixOfCFandFF(DriverUpdateParams& p,
		double targetSpeed, double maxLaneSpeed) {
	if (p.space > p.distanceToNormalStop) {
		return accOfFreeFlowing(p, targetSpeed, maxLaneSpeed);
	} else {
//		double dt	=	p.elapsedSeconds;
		double dt = p.nextStepSize;
		//p.space_star	=	p.space + p.v_lead * dt + 0.5 * p.a_lead * dt * dt;
		double s = p.space_star;
		double v = p.v_lead + p.a_lead * dt;
		return brakeToTargetSpeed(p, s, v);
	}
}

void sim_mob::MITSIM_CF_Model::distanceToNormalStop(DriverUpdateParams& p) {
//	double minSpeed = 0.1;
//	double minResponseDistance = 5;
//	double DIS_EPSILON = 0.001;
	if (p.perceivedFwdVelocity / 100 > minSpeed) {
		p.distanceToNormalStop = sim_mob::Math::DOUBLE_EPSILON
				- 0.5 * (p.perceivedFwdVelocity / 100)
						* (p.perceivedFwdVelocity / 100) / p.normalDeceleration;
		if (p.distanceToNormalStop < minResponseDistance) {
			p.distanceToNormalStop = minResponseDistance;
		}
	} else {
		p.distanceToNormalStop = minResponseDistance;
	}
}
void sim_mob::MITSIM_CF_Model::calcStateBasedVariables(DriverUpdateParams& p) {
	distanceToNormalStop(p);

	// Acceleration rate for a vehicle (a function of vehicle type,
	// facility type, segment grade, current speed).
	p.maxAcceleration = getMaxAcceleration(p);
	// Deceleration rate for a vehicle (a function of vehicle type, and
	// segment grade).
	p.normalDeceleration = getNormalDeceleration(p);
	// Maximum deceleration is function of speed and vehicle class
	p.maxDeceleration = getMaxDeceleration(p);

	// unsetStatus;
	p.unsetStatus(STATUS_REGIME);
}
void sim_mob::MITSIM_CF_Model::calcUpdateStepSizes() {
	// dec
	double totalReactionTime = makeNormalDist(decUpdateStepSize);
	double perceptionTime = totalReactionTime * decUpdateStepSize.percep; // perception time  = reaction time * percentage
//	double engagementTime = totalReactionTime * (1-decUpdateStepSize.percep);
	updateStepSize.push_back(totalReactionTime);
	perceptionSize.push_back(perceptionTime);
	// acc
	totalReactionTime = makeNormalDist(accUpdateStepSize);
	perceptionTime = totalReactionTime * accUpdateStepSize.percep; // perception time  = reaction time * percentage
//	engagementTime = totalReactionTime * (1-accUpdateStepSize.percep);
	updateStepSize.push_back(totalReactionTime);
	perceptionSize.push_back(perceptionTime);
	// uniform Speed
	totalReactionTime = makeNormalDist(uniformSpeedUpdateStepSize);
	perceptionTime = totalReactionTime * uniformSpeedUpdateStepSize.percep; // perception time  = reaction time * percentage
//	engagementTime = totalReactionTime * (1-uniformSpeedUpdateStepSize.percep);
	updateStepSize.push_back(totalReactionTime);
	perceptionSize.push_back(perceptionTime);
	// stopped vehicle
	totalReactionTime = makeNormalDist(stoppedUpdateStepSize);
	perceptionTime = totalReactionTime * stoppedUpdateStepSize.percep; // perception time  = reaction time * percentage
//	engagementTime = totalReactionTime * (1-stoppedUpdateStepSize.percep);
	updateStepSize.push_back(totalReactionTime);
	perceptionSize.push_back(perceptionTime);
}
double sim_mob::MITSIM_CF_Model::makeNormalDist(UpdateStepSizeParam& sp) {
	boost::normal_distribution<double> nor(sp.mean, sp.stdev);
	boost::variate_generator<boost::mt19937, boost::normal_distribution<double> > dice(
			updateSizeRm, nor);
	double v = dice();
	//TODO need use truncated log distribution
	if (v < sp.lower)
		return sp.lower;
	if (v > sp.upper)
		return sp.upper;
	return v;
}
double sim_mob::CarFollowModel::calcNextStepSize(DriverUpdateParams& p) {
	double accRate_ = p.driver->fwdAccel / 100.0;
	double currentSpeed_ = p.driver->fwdVelocity / 100.0;

	int i;
	if (accRate_ < -sim_mob::Math::DOUBLE_EPSILON)
		i = 0;
	else if (accRate_ > sim_mob::Math::DOUBLE_EPSILON)
		i = 1;
	else if (currentSpeed_ > sim_mob::Math::DOUBLE_EPSILON)
		i = 2;
	else
		i = 3;
	p.nextStepSize = updateStepSize[i];
	nextPerceptionSize = perceptionSize[i];
	p.driver->resetReacTime(nextPerceptionSize * 100);
	return p.nextStepSize;
}
