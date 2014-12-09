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
#include "entities/roles/driver/models/CarFollowModel.hpp"
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

double convertFrmMillisecondToSecond(double v) {
	return v/1000.0;
}

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
{
	modelName = "general_driver_model";
	splitDelimiter = " ,";
	initParam(p);
}

void sim_mob::MITSIM_CF_Model::initParam(sim_mob::DriverUpdateParams& p) {

	// speed scaler
	string speedScalerStr, maxAccStr, decelerationStr, maxAccScaleStr,
			normalDecScaleStr, maxDecScaleStr;
	bool isAMOD = false;

	if (p.driver->getParent()->amodId != "-1")
	{
		isAMOD = true;
	}

	ParameterManager *parameterMgr = ParameterManager::Instance(isAMOD);

	parameterMgr->param(modelName, "speed_scaler",
			speedScalerStr, string("5 20 20"));

	// max acceleration
	parameterMgr->param(modelName, "max_acc_car1", maxAccStr,
			string("10.00  7.90  5.60  4.00  4.00"));
	makeSpeedIndex(Vehicle::CAR, speedScalerStr, maxAccStr, maxAccIndex,
			maxAccUpperBound);
	parameterMgr->param(modelName, "max_acceleration_scale",
			maxAccScaleStr, string("0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5"));
	makeScaleIdx(maxAccScaleStr, maxAccScale);

	// normal deceleration
	parameterMgr->param(modelName, "normal_deceleration_car1",
			decelerationStr, string("7.8 	6.7 	4.8 	4.8 	4.8"));
	makeSpeedIndex(Vehicle::CAR, speedScalerStr, decelerationStr,
			normalDecelerationIndex, normalDecelerationUpperBound);
	parameterMgr->param(modelName, "normal_deceleration_scale",
			normalDecScaleStr,
			string("1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0"));
	makeScaleIdx(maxAccScaleStr, normalDecelerationScale);

	// speed limit add on
	string str;
	parameterMgr->param(modelName, "speed_limit_add_on", str,
			string("-0.1911 -0.0708 -0.0082 0.0397 0.0810 0.1248 0.1661 0.2180 0.2745 0.3657"));
	makeScaleIdx(str, speedLimitAddon);

	// acc add on
	parameterMgr->param(modelName, "Car_following_acceleration_add_on", str,
				string("-1.3564 -0.8547 -0.5562 -0.3178 -0.1036 0.1036 0.3178 0.5562 0.8547 1.3564"));
	sim_mob::Utils::convertStringToArray(str,accAddon);

	// decl add on
	parameterMgr->param(modelName, "Car_following_deceleration_add_on", str,
					string("-1.3187 -0.8309 -0.5407 -0.3089 -0.1007 0.1007 0.3089 0.5407 0.8309 1.3187"));
	sim_mob::Utils::convertStringToArray(str,declAddon);

	// max deceleration
	parameterMgr->param(modelName, "max_deceleration_car1",
			decelerationStr, string("-16.0   -14.5   -13.0   -11.0   -9.0"));
	makeSpeedIndex(Vehicle::CAR, speedScalerStr, decelerationStr,
			maxDecelerationIndex, maxDecelerationUpperBound);
	parameterMgr->param(modelName, "max_deceleration_scale",
			maxDecScaleStr, string("1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0"));
	makeScaleIdx(maxDecScaleStr, maxDecelerationScale);

	// acceleration grade factor
	parameterMgr->param(modelName, "acceleration_grade_factor",
			accGradeFactor, 0.305);
	parameterMgr->param(modelName, "tmp_all_grades", tmpGrade,
			0.0);

	// param for distanceToNormalStop()
	parameterMgr->param(modelName, "min_speed", minSpeed, 0.1);
	parameterMgr->param(modelName, "min_response_distance",
			minResponseDistance, 5.0);

	// param of calcSignalRate()
	parameterMgr->param(modelName, "yellow_stop_headway",
			yellowStopHeadway, 1.0);
	parameterMgr->param(modelName, "min_speed_yellow",
			minSpeedYellow, 2.2352);

	// param of carFollowingRate()
	parameterMgr->param(modelName, "hbuffer_lower",
			hBufferLower, 0.8);
	string hBufferUpperStr;
	parameterMgr->param(modelName, "hbuffer_Upper",
			hBufferUpperStr,
			string("1.7498 2.2737 2.5871 2.8379 3.0633 3.2814 3.5068 3.7578 4.0718 4.5979"));
	makeScaleIdx(hBufferUpperStr, hBufferUpperScale);
	hBufferUpper = getBufferUppder();

	// Car following parameters
	string cfParamStr;
	parameterMgr->param(modelName, "CF_parameters_1",
			cfParamStr,
			string("0.0400, 0.7220, 0.2420, 0.6820, 0.6000, 0.8250"));
	makeCFParam(cfParamStr, CF_parameters[0]);
	parameterMgr->param(modelName, "CF_parameters_2",
			cfParamStr, string("-0.0418 0.0000 0.1510 0.6840 0.6800 0.8020"));
	makeCFParam(cfParamStr, CF_parameters[1]);

	string targetGapAccParmStr;
	parameterMgr->param(modelName, "target_gap_acc_parm",
			targetGapAccParmStr,
			string(
					"0.604, 0.385, 0.323, 0.0678, 0.217,0.583, -0.596, -0.219, 0.0832, -0.170, 1.478, 0.131, 0.300"));
	makeScaleIdx(targetGapAccParmStr, targetGapAccParm);

	//UPDATE STEP SIZE (REACTION TIME RELATED)
	string updateStepSizeStr;

	// dec
	parameterMgr->param(modelName, "dec_update_step_size",
			updateStepSizeStr, string("0.5     0.0     0.5     0.5 0.5"));
	makeUpdateSizeParam(updateStepSizeStr, decUpdateStepSize);

	//speed factor
	parameterMgr->param(modelName, "speed_factor", speedFactor,
			1.0);

	// acc
	parameterMgr->param(modelName, "acc_update_step_size",
			updateStepSizeStr, string("1.0     0.0     1.0     1.0 0.5"));
	makeUpdateSizeParam(updateStepSizeStr, accUpdateStepSize);

	// uniform speed
	parameterMgr->param(modelName,
			"uniform_speed_update_step_size", updateStepSizeStr,
			string("1.0     0.0     1.0     1.0 0.5"));
	makeUpdateSizeParam(updateStepSizeStr, uniformSpeedUpdateStepSize);

	// stopped vehicle
	parameterMgr->param(modelName,
			"stopped_vehicle_update_step_size", updateStepSizeStr,
			string("0.5     0.0     0.5     0.5 0.5"));
	makeUpdateSizeParam(updateStepSizeStr, stoppedUpdateStepSize);

	boost::random_device seed_gen;
	long int r = seed_gen();
	updateSizeRm = boost::mt19937(r);
	calcUpdateStepSizes();

	// init step size , i=3 stopped vehicle
	p.nextStepSize = updateStepSize[3];
	if(p.nextStepSize == 0){
		p.nextStepSize = p.elapsedSeconds;
	}
	nextPerceptionSize = perceptionSize[3];

	// visibility
	parameterMgr->param(modelName, "visibility_distance",
			visibilityDistance, 10.0);

	//FF Acc Params
	parameterMgr->param(modelName,
					"FF_Acc_Params_b2", p.FFAccParamsBeta,
					0.3091);

	//density
	parameterMgr->param(modelName,
						"density", p.density,
						40.0);

	// driver signal perception distance
	parameterMgr->param(modelName,
							"driver_signal_perception_distance", percepDisM,
							75.0);
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

	boost::algorithm::trim(cstr);
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

double sim_mob::MITSIM_CF_Model::getDeclAddon()
{
	int scaleNo = Utils::generateInt(1, declAddon.size() - 1);
	double res = Utils::generateFloat(declAddon[scaleNo-1],declAddon[scaleNo]);
	return res;
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

	calcStateBasedVariables(p);
	p.desiredSpeed = calcDesiredSpeed(p);

	double acc = p.maxAcceleration; p.accSelect = "max";
	double aB = calcMergingRate(p);
	double aC = calcSignalRate(p); // near signal or incidents
	double aD = calcYieldingRate(p); // when yielding
	//double aE = waitExitLaneRate(p); //
	double aF = waitAllowedLaneRate(p);
	//double  aG = calcLaneDropRate(p);	// MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)
	//double aH1 = calcAdjacentRate(p); // to reach adjacent gap
	//double aH2 = calcBackwardRate(p); // to reach backward gap
	//double aH3 = calcForwardRate(p);  // to reach forward gap
	// The target gap acceleration should be based on the target gap status and not on the min
	// MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)
	double aH = p.maxAcceleration;
	std::string aHStr="aH";
	if (!p.getStatus(STATUS_LC_CHANGING)) // not in middle of performing lane change
	{
		if (p.getStatus(STATUS_ADJACENT))
		{
			aH = calcAdjacentRate(p);	// to reach adjacent gap
			aHStr = "aHA";
		}
		else if (p.getStatus(STATUS_BACKWARD))
		{
			aH = calcBackwardRate(p);	// to reach backward gap
			aHStr = "aHB";
		}
		else if (p.getStatus(STATUS_FORWARD))
		{
			aH = calcForwardRate(p);		// to reach forward gap
			aHStr = "aHF";
		}
		else
		{
			aH = desiredSpeedRate(p);
			aHStr = "aHD";
		}
	}

	// if (intersection){
	// double aI = approachInter(p); // when approaching intersection to achieve the turn speed
	// if(acc > aI) acc = aI;
	// }
	// FUNCTION approachInter MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)

	//double aZ1 = carFollowingRate(p, p.nvFwd);
	//double aZ2 = carFollowingRate(p, p.nvFwdNextLink);
	double aZ = calcCarFollowingRate(p);
	p.aZ = aZ;

	// stop point acc
	double aSP = calcStopPointRate(p);

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
		p.accSelect = aHStr;
	}
	//if (acc > aH1)
	//acc = aH1;
	//if (acc > aH2)
	//acc = aH2;
	//if (acc > aH3)
	//acc = aH3;
	//if(acc > aG) acc = aG;
	if (acc > aC) {
		acc = aC;
		p.accSelect = "aC";
	}
	//if (acc > aE)
	//acc = aE;
	if (acc > aZ) {
		acc = aZ;
		p.accSelect = "aZ";
	}

	if (acc>aSP){
		acc = aSP;
		p.accSelect = "aSP";
	}
	//if (acc > aZ1)
	//acc = aZ1;
	//if (acc > aZ2)
	//acc = aZ2;

	// SEVERAL CONDITONS MISSING! > NOT YET IMPLEMENTED (@CLA_04/14)

	p.lastAcc = acc;

	// if brake ,alerm follower
	if (acc < -ACC_EPSILON) {

		// I am braking, alert the vehicle behind if it is close
		if (p.nvBack.exists() )
		{
			Driver* bvd = const_cast<Driver*>(p.nvBack.driver);
			DriverUpdateParams& bvp = bvd->getParams();
			if( p.nvBack.distance < visibility() &&
					!(bvd->isBus() && bvp.getStatus(STATUS_STOPPED))) {
						  float alert = CF_CRITICAL_TIMER_RATIO * updateStepSize[0];
						  bvp.cftimer = std::min<double>(alert,bvp.cftimer);
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

	std::stringstream debugStr;
	debugStr<<"t"<<p.now.frame();

	//If we have no space left to move, immediately cut off acceleration.
	p.headway = 99;
	if (p.perceivedDistToFwdCar == DEFAULT_DISTANCE_CM)
	{
		res = accOfFreeFlowing(p, p.desiredSpeed, p.maxLaneSpeed);
		debugStr << "DEF;" << res;
	}
	else
	{
		debugStr << "ELSE;";
		// when nv is left/right vh , can not use perceivedxxx!
		// create perceived left,right varialbe
		p.v_lead = p.perceivedFwdVelocityOfFwdCar / 100;
		p.a_lead = p.perceivedAccelerationOfFwdCar / 100;

		double dt = p.nextStepSize;
		float auxspeed =
				p.perceivedFwdVelocity / 100 == 0 ?
						0.00001 : p.perceivedFwdVelocity / 100;

		float headway = 2.0 * p.space
				/ (auxspeed + p.perceivedFwdVelocity / 100);

		debugStr << "+" << headway << "+" << p.space << "+" << auxspeed << "+"
				<< p.perceivedFwdVelocity << ";";

		double emergSpace = nv.distance / 100;

		debugStr << emergSpace << ";";

		// to fix bug: when subject vh speed=0 and space small, headway become large number
		p.emergHeadway = -1;
		if (emergSpace < 2.0)
		{
			double speed = p.perceivedFwdVelocity / 100;
			double emergHeadway = CalcHeadway(emergSpace, speed,
					p.elapsedSeconds, p.maxAcceleration);
			if (emergHeadway < hBufferLower)
			{
				//We need to brake. Override.
				p.space = emergSpace;
				headway = emergHeadway;
			}
			p.emergHeadway = emergHeadway;

			debugStr << "EM;" << emergHeadway << ";";
		}

		float v = p.v_lead + p.a_lead * dt;
		p.space_star = p.space + 0.5 * (p.v_lead + v) * dt;
		if (headway < hBufferLower)
		{
			res = accOfEmergencyDecelerating(p);
			p.setStatus(STATUS_REGIME_EMERGENCY);
			debugStr << "LO;";
		}
		hBufferUpper = getBufferUppder();
		if (headway > hBufferUpper)
		{
			res = accOfMixOfCFandFF(p, p.desiredSpeed, p.maxLaneSpeed);
			debugStr << "UP;";
		}
		if (headway <= hBufferUpper && headway >= hBufferLower)
		{
			res = accOfCarFollowing(p);
			debugStr << "LOUP;";
		}

		p.headway = headway;

	}	//end of else

	return res;
}

double sim_mob::MITSIM_CF_Model::calcCarFollowingRate(DriverUpdateParams& p)
{
	//status_ &= ~(STATUS_REGIME);
	//need unset STATUS_REGIME?
	double acc;

	//isInMergingArea() function is not implemented
	/*if(isInMergingArea(p))
	{

	}
	else*/
	{
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
}

double sim_mob::MITSIM_CF_Model::calcMergingRate(
		sim_mob::DriverUpdateParams& p)
{
	double acc = p.maxAcceleration;
	// TS_Vehicles from freeways and on ramps have different
	// priority.  Seperate procedures are applied. (MITSIM TS_CFModels.cc)
	DriverMovement *driverMvt = (DriverMovement*) p.driver->Movement();

	if (driverMvt->fwdDriverMovement.getCurrSegment()->type
			== sim_mob::LINK_TYPE_FREEWAY) // current on freeway
	{
		if (p.nvLeadFreeway.exists()) // has lead vh on next link
		{
			double headway = headwayBuffer(); //TODO: headway is correct?
			if (p.nvLeadFreeway.distance / 100.0 < headway)
			{
				// MITSIM TS_CFModels.cc
				acc = p.nvLeadFreeway.driver->fwdAccel.get() / 100.0
						+ brakeToTargetSpeed(p,
								p.nvLeadFreeway.distance / 100.0,
								p.nvLeadFreeway.driver->fwdVelocity);
			}
		}
	}
	else if (driverMvt->fwdDriverMovement.getCurrSegment()->type
			== sim_mob::LINK_TYPE_RAMP) // on ramp
	{
		if (p.nvLagFreeway.exists()) // has lag vh on freeway
		{
			if (!isGapAcceptable(p, p.nvLagFreeway))
			{
				// The gap is not acceptable. Prepare to stop.
				return brakeToStop(p, p.dis2stop);
			}
		}
		else
		{
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
	float speedM = vh.driver->fwdVelocity / 100.0; //coming->currentSpeed_;

	// get object vh's max acceleration
	Driver* d = const_cast<Driver*>(vh.driver);
	DriverUpdateParams& dp = d->getParams();
	float accm = dp.maxAcceleration; //coming->maxAcceleration();
	float speedn;

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

	float dism = speedM * dt + 0.5 * accm * dt * dt;
	float gap_mn = vh.distance / 100.0;

	// Speed at the predicted position

	speedM += accm * dt;
	float sd = (speedM - speedn) * headwayBuffer();
	float threshold = (sd > 0.0) ? sd : 0.0;

	// check if the gap is acceptable

	if (gap_mn > threshold) {
		return true;
	}
	else {
		return 0;
	}
}

double sim_mob::MITSIM_CF_Model::calcSignalRate(DriverUpdateParams& p) {
	double minacc = p.maxAcceleration;

	sim_mob::TrafficColor color;

#if 0
	Signal::TrafficColor color;
#endif
	double distanceToTrafficSignal;
	distanceToTrafficSignal = p.perceivedDistToTrafficSignal;
	color = p.perceivedTrafficColor;

	if (distanceToTrafficSignal < percepDisM * 100)
	{
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
		}
		else if (color == sim_mob::Amber)
#if 0
		else if(color == Signal::Amber)
#endif
		{
			double maxSpeed =
					(p.perceivedFwdVelocity / 100 > minSpeedYellow) ?
							p.perceivedFwdVelocity / 100 : minSpeedYellow;
			if (dis / maxSpeed > yellowStopHeadway)
			{
				double a = brakeToStop(p, dis);
				if (a < minacc)
					minacc = a;
			}
		}
		else if (color == sim_mob::Green)
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
	p.lcDebugStr<<";---CYR";
	if (p.flag(FLAG_YIELDING)) {
		p.lcDebugStr<<";DING";
		// Make sure a vehicle will not yield infinitely.
		uint32_t dt_sec = convertFrmMillisecondToSecond(p.now.ms() - p.yieldTime.ms());
		p.lcDebugStr<<";dt"<<dt_sec;
		if (dt_sec > p.lcMaxNosingTime) {
			p.driver->yieldVehicle = NULL;
			p.unsetFlag(FLAG_YIELDING);
			p.lcDebugStr<<";yd1";
			return p.maxAcceleration;
		} //end of lcMaxNosingTime

		// This vehicle is yielding to another vehicle

		bool rightFwdVhFlag = false;
		if (p.nvRightFwd.exists()) {
			p.lcDebugStr<<";yd2";
			Driver* dRF = const_cast<Driver*>(p.nvRightFwd.driver);
			DriverUpdateParams& pRF = dRF->getParams();
			if (pRF.flag(FLAG_NOSING_LEFT)) {
				rightFwdVhFlag = true;
				p.lcDebugStr<<";yd3";
			}
		}

		bool leftFwdVhFlag = false;
		if (p.nvLeftFwd.exists()) {
			p.lcDebugStr<<";yd4";
			Driver* d = const_cast<Driver*>(p.nvLeftFwd.driver);
			DriverUpdateParams& p = d->getParams();
			if (p.flag(FLAG_NOSING_RIGHT)) {
				leftFwdVhFlag = true;
				p.lcDebugStr<<";yd5";
			}
		}

		if (p.flag(FLAG_YIELDING_RIGHT)) {
			p.lcDebugStr<<";yd6";
			if ((p.rightLane) && // right side has lane
					(p.nvRightFwd.exists()) && // right lane has fwd vh
					p.nvRightFwd.driver == p.driver->yieldVehicle && // the right fwd vh is nosing
					rightFwdVhFlag // right fwd vh nosing
					) {
				p.lcDebugStr<<";yd7";
				acc = carFollowingRate(p, p.nvRightFwd);
				if (acc < p.normalDeceleration) {
					acc = p.normalDeceleration;
				} else if (acc > 0) {
					acc = 0.0;
				}
				p.lcDebugStr<<";acc"<<acc;
				return acc;
			}
		} else if (p.flag(FLAG_YIELDING_LEFT)) {
			p.lcDebugStr<<";yd8";
			if ((p.leftLane) && // left side has lane
					(p.nvLeftFwd.exists()) && // left lane has fwd vh
					p.nvLeftFwd.driver == p.driver->yieldVehicle && // the left fwd vh is nosing
					leftFwdVhFlag) {
				p.lcDebugStr<<";yd9";
				acc = carFollowingRate(p, p.nvLeftFwd);
				if (acc < p.normalDeceleration) {
					acc = p.normalDeceleration;
				} else if (acc > 0) {
					acc = 0.0;
				}
				p.lcDebugStr<<";acc"<<acc;
				return acc;
			}
		} // end of else

		p.driver->yieldVehicle = NULL;
		p.unsetFlag(FLAG_YIELDING);
		p.lcDebugStr<<";yd10"<<acc;
		return p.maxAcceleration;

	} //end if flag(FLAG_YIELDING)
	else if (p.flag(FLAG_NOSING)) {
		p.lcDebugStr<<";SING";
		// This vehicle is nosing
		bool rightBackVhFlag = false;
		if (p.nvRightBack.exists()) {
			Driver* d = const_cast<Driver*>(p.nvRightBack.driver);
			DriverUpdateParams& pd = d->getParams();
			if (pd.flag(FLAG_YIELDING_LEFT) || pd.flag(FLAG_NOSING)) {
				rightBackVhFlag = true;
			}
		}
		bool leftBackVhFlag = false;
		if (p.nvLeftBack.exists()) {
			Driver* d = const_cast<Driver*>(p.nvLeftBack.driver);
			DriverUpdateParams& pd = d->getParams();
			if (pd.flag(FLAG_YIELDING_RIGHT) || pd.flag(FLAG_NOSING)) {
				leftBackVhFlag = true;
			}
		}

		if (p.flag(FLAG_NOSING_RIGHT)) {
			p.lcDebugStr<<";RT";
			p.lcDebugStr<<";RBD"<<p.nvRightBack.distance/100.0;
			p.lcDebugStr<<";RFD"<<p.nvRightFwd.distance/100.0;
			if ((p.rightLane) && // has right lane
					(p.nvRightBack.exists()) && // has right back vh
					rightBackVhFlag) // right back vh yielding left
			{
				acc = calcCreateGapRate(p, p.nvRightFwd,
						p.lcMinGap(2) + Math::DOUBLE_EPSILON); //other->vehicleAhead(),theParameter->lcMinGap(2) + DIS_EPSILON);
				double res = std::max<double>(p.maxDeceleration, acc);
				p.lcDebugStr<<";acc"<<acc;
				return res;
			}
		} else if (p.flag(FLAG_NOSING_LEFT)) {
			p.lcDebugStr<<";LT";
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
		p.lcDebugStr<<";NTH";
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
	float dx = vh.distance/100.0 - gap; //gapDistance(front) - gap;
	float dv = p.currSpeed - vh.driver->fwdVelocity / 100.0;

	float dt = p.nextStepSize;
	if (dt <= 0.0)
		return p.maxAcceleration;
#if 0
	double res = vh.driver->fwdAccel.get()/100.0 + 2.0 * (dx - dv * dt) / (dt * dt);
	return res;
#else
	if (dx < 0.01 || dv < 0.0)
	{
		// insufficient gap or my speed is slower than the leader
		//front->accRate_ + 2.0 * (dx - dv * dt) / (dt * dt);
		double res = vh.driver->fwdAccel.get() / 100.0
				+ 2.0 * (dx - dv * dt) / (dt * dt);
		return res;
	}
	else
	{
		// gap is ok and my speed is higher.
		//front->accRate_ - 0.5 * dv * dv / dx;
		double res = vh.driver->fwdAccel.get() / 100.0 - 0.5 * dv * dv / dx;
		return res;
	}
#endif
}

double sim_mob::MITSIM_CF_Model::waitExitLaneRate(DriverUpdateParams& p) {

	// dis2stop is distance to fwd vh or distance to end node
	DriverMovement *driverMvt = (DriverMovement*)p.driver->Movement();
	double dx = driverMvt->fwdDriverMovement.getDistToLinkEndM() - 5.0;

	if(!p.getStatus(STATUS_CURRENT_LANE_OK) && dx < p.distanceToNormalStop)
	{
		return brakeToStop(p,dx);
	}
	else
	{
		return p.maxAcceleration;
	}
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
	const NearestVehicle * av = NULL; // side leader vh

	if (p.getStatus(STATUS_LEFT))
	{
		av = &p.nvLeftFwd;
	}
	else if (p.getStatus(STATUS_RIGHT))
	{
		av = &p.nvRightFwd;
	}
	else
	{
		// No request for lane change
		return p.maxAcceleration;
	}

	std::vector<double> a = targetGapAccParm;

	double dis;
	double dv;
	if (av->exists())
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

	if (dv > 0)
	{
		acc *= pow(dv, a[3]);
	}
	else if (dv < 0)
	{
		acc *= pow(-dv, a[4]);
	}

	acc += getAccAddon() * a[5] / 0.824;
	return acc;
}

double sim_mob::MITSIM_CF_Model::calcBackwardRate(DriverUpdateParams& p) {

	const NearestVehicle * bv = NULL; // side follower vh

	if (p.getStatus(STATUS_LEFT)) {
		bv = &p.nvLeftBack;
	} else if (p.getStatus(STATUS_RIGHT)) {
		bv = &p.nvRightBack;
	} else {
		// No request for lane change
		return p.maxAcceleration;
	}

	std::vector<double> a = targetGapAccParm;

	double dis;
	double dv;

	if (bv->exists())
	{
		Driver *bvDriver = const_cast<Driver*>(bv->driver);

		dis = bv->distance + a[0];
		dv = bvDriver->getFwdVelocityM() - p.driver->getFwdVelocityM();
	}
	else
	{
		dis = bv->distance + a[0];
		dv = 0 - p.driver->getFwdVelocityM();
	}

	float acc = a[6] * pow(dis, a[7]);

	if (dv > 0) {
	acc *= pow(dv, a[8]);
	} else if (dv < 0) {
	acc *= pow (-dv, a[9]);
	}

	acc += getAccAddon() *  a[10] / 0.824 ;
	return acc;
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
		  // No request for lane change
		  return p.maxAcceleration;
	  }

	  std::vector<double> a = targetGapAccParm;

	 if (!av->exists()) return p.maxAcceleration;

	 if (!bv->exists()) return p.maxAcceleration;

	 Driver *avDriver = const_cast<Driver*>(av->driver);
	 Driver *bvDriver = const_cast<Driver*>(bv->driver);

	 float gap = bvDriver->gapDistance(avDriver);
	 float position = bvDriver->gapDistance(p.driver)+ p.driver->getVehicleLengthM();
	 float acc = a[11] * (a[0] * gap - position);

	 acc += getAccAddon() * a[12] / 0.824 ;
	 p.lcDebugStr<<"+++acc+++"<<acc;
	 return acc;
}

double sim_mob::MITSIM_CF_Model::calcStopPointRate(sim_mob::DriverUpdateParams& p){
	if(p.parentId == 664 && p.now.frame()>751){
		int i=0;
	}
	std::stringstream debugStr;
	debugStr<<";SSPP"<<p.disToSP<<";"<<p.stopPointState<<";";
	double acc=p.maxAcceleration;
	if(!p.getStatus(STATUS_CHANGING)){
		if(p.stopPointState == DriverUpdateParams::CLOSE_STOP_POINT){
			acc = brakeToStop(p, p.dis2stop);
			debugStr<<"SP-Close;";
			p.cfDebugStr += debugStr.str();
			return acc;
		}
		if(p.stopPointState == DriverUpdateParams::JUST_ARRIVE_STOP_POINT || p.stopPointState == DriverUpdateParams::WAITING_AT_STOP_POINT){
			debugStr<<"SP-Arrive;";
			acc = -10;
		}// end of stopPointState
	}
	if(p.stopPointState == DriverUpdateParams::JUST_ARRIVE_STOP_POINT && p.perceivedFwdVelocity / 100 < 0.1){
		debugStr<<"SP-Arrive0;";
		acc = -10;
		p.stopPointState = DriverUpdateParams::WAITING_AT_STOP_POINT;
		p.startStopTime = p.now.ms();
	}
	if(p.stopPointState == DriverUpdateParams::WAITING_AT_STOP_POINT){
		debugStr<<"SP-Waiting;";
		double currentTime = p.now.ms();
		double t = (currentTime - p.startStopTime) / 1000.0;// convert ms to s

		debugStr<<"SPt;"<<t;

		if(t>p.currentStopPoint.dwellTime){
			p.stopPointState = DriverUpdateParams::LEAVING_STOP_POINT;
		}
	}
	p.cfDebugStr += debugStr.str();
	return acc;
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

	if (dis > sim_mob::Math::DOUBLE_EPSILON)
	{
		double u2 = (p.perceivedFwdVelocity / 100)
				* (p.perceivedFwdVelocity / 100);
		double acc = -u2 / dis * 0.5;

		if (acc <= p.normalDeceleration)
		{
			return acc;
		}

		double dt = p.nextStepSize;
		double vt = p.perceivedFwdVelocity / 100 * dt;
		double a = dt * dt;
		double b = 2.0 * vt - p.normalDeceleration * a;
		double c = u2 + 2.0 * p.normalDeceleration * (dis - vt);
		double d = b * b - 4.0 * a * c;

		if (d < 0 || a <= 0.0)
		{
			return acc;
		}

		return (sqrt(d) - b) / a * 0.5;
	}
	else
	{
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
  float maxspd = p.maxLaneSpeed;
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
	double dt = p.nextStepSize;
	double currentSpeed_ = p.perceivedFwdVelocity / 100;

	if (s > sim_mob::Math::DOUBLE_EPSILON) {
		float v2 = v * v;
		float u2 = currentSpeed_ * currentSpeed_;
		float acc = (v2 - u2) / s * 0.5;

		return acc;
	} else {
		//	float dt = nextStepSize;
		//if (dt <= 0.0) return maxAcceleration ;
		return (v - currentSpeed_) / dt;
	}
}

double sim_mob::MITSIM_CF_Model::accOfEmergencyDecelerating(
		DriverUpdateParams& p) {
	double v = p.perceivedFwdVelocity / 100;
	double dv = v - p.v_lead;
	double epsilon_v = sim_mob::Math::DOUBLE_EPSILON;
	if(v<epsilon_v) return 0;
	double aNormalDec = p.normalDeceleration;

	double a;
	if (dv < epsilon_v) {
		a = p.a_lead + 0.25 * aNormalDec;
	} else if (p.space > 0.01) {
		a = p.a_lead - dv * dv / 2 / p.space;
	} else {
		//double dt	=	p.elapsedSeconds;
		double dt = p.nextStepSize;
		//p.space_star	=	p.space + p.v_lead * dt + 0.5 * p.a_lead * dt * dt;
		double s = p.space_star;
		double v = p.v_lead + p.a_lead * dt;
		a = brakeToTargetSpeed(p, s, v);
	}
	return min(p.normalDeceleration, a);
}

double sim_mob::MITSIM_CF_Model::accOfCarFollowing(DriverUpdateParams& p) {

	double density = p.density;
	double v = p.perceivedFwdVelocity / 100;
	int i = (v > p.v_lead) ? 1 : 0;
	double dv = (v > p.v_lead) ? (v - p.v_lead) : (p.v_lead - v);

	double res = CF_parameters[i].alpha * pow(v, CF_parameters[i].beta)
			/ pow(p.nvFwd.distance / 100, CF_parameters[i].gama);

	double t0 = pow(p.nvFwd.distance / 100, CF_parameters[i].gama);

	double t1 = pow(v, CF_parameters[i].beta);

	double tt = pow(dv, CF_parameters[i].lambda);

	double t2 = pow(density, CF_parameters[i].rho);

	res *= pow(dv, CF_parameters[i].lambda)
			* pow(density, CF_parameters[i].rho);

	res += feet2Unit(Utils::nRandom(0, CF_parameters[i].stddev));

	return res;
}

double sim_mob::MITSIM_CF_Model::accOfFreeFlowing(DriverUpdateParams& p,
		double targetSpeed, double maxLaneSpeed) {
	double vn = p.perceivedFwdVelocity / 100;
	if (vn < targetSpeed - minSpeed) {
		double acc = p.FFAccParamsBeta * (targetSpeed-vn); //+ getAccAddon();
		return acc;
	} else if (vn > targetSpeed + minSpeed) {
		return p.normalDeceleration;
	}
	//If equal:
	return 0;
}

double sim_mob::MITSIM_CF_Model::accOfMixOfCFandFF(DriverUpdateParams& p,
		double targetSpeed, double maxLaneSpeed) {
	if (p.space > p.distanceToNormalStop) {
		return accOfFreeFlowing(p, targetSpeed, maxLaneSpeed);
	}
	else {
		double dt = p.nextStepSize;
		//p.space_star	=	p.space + p.v_lead * dt + 0.5 * p.a_lead * dt * dt;
		double s = p.space_star;
		double v = p.v_lead + p.a_lead * dt;
		return brakeToTargetSpeed(p, s, v);
	}
}

void sim_mob::MITSIM_CF_Model::distanceToNormalStop(DriverUpdateParams& p) {

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

	updateStepSize.push_back(totalReactionTime);
	perceptionSize.push_back(perceptionTime);

	// acc
	totalReactionTime = makeNormalDist(accUpdateStepSize);
	perceptionTime = totalReactionTime * accUpdateStepSize.percep; // perception time  = reaction time * percentage

	updateStepSize.push_back(totalReactionTime);
	perceptionSize.push_back(perceptionTime);

	// uniform Speed
	totalReactionTime = makeNormalDist(uniformSpeedUpdateStepSize);
	perceptionTime = totalReactionTime * uniformSpeedUpdateStepSize.percep; // perception time  = reaction time * percentage

	updateStepSize.push_back(totalReactionTime);
	perceptionSize.push_back(perceptionTime);

	// stopped vehicle
	totalReactionTime = makeNormalDist(stoppedUpdateStepSize);
	perceptionTime = totalReactionTime * stoppedUpdateStepSize.percep; // perception time  = reaction time * percentage

	updateStepSize.push_back(totalReactionTime);
	perceptionSize.push_back(perceptionTime);
}

double sim_mob::MITSIM_CF_Model::makeNormalDist(UpdateStepSizeParam& sp) {

	if(sp.mean == 0)
	{
		return 0;
	}//boost::normal_distribution<double> nor(sp.mean, sp.stdev);
	boost::lognormal_distribution<double> nor(sp.mean, sp.stdev);
	boost::variate_generator<boost::mt19937, boost::lognormal_distribution<double> > dice(
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
	if(currentSpeed_ <  minSpeed)
		i = 3;
	else {
		if (accRate_ < -sim_mob::Math::DOUBLE_EPSILON)
			i = 0;
		else if (accRate_ > sim_mob::Math::DOUBLE_EPSILON)
			i = 1;
		else if (currentSpeed_ > sim_mob::Math::DOUBLE_EPSILON)
			i = 2;
		else
			i = 3;
	}

	p.nextStepSize = updateStepSize[i];
	if(p.nextStepSize == 0){
		p.nextStepSize = p.elapsedSeconds;
	}
	nextPerceptionSize = perceptionSize[i];
	p.driver->resetReacTime(nextPerceptionSize * 1000);
	return p.nextStepSize;
}
