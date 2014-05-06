//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "conf/params/ParameterManager.hpp"
#include "entities/roles/driver/Driver.hpp"

#include <string>
#include <set>
#include <map>
#include <boost/random.hpp>

using namespace std;

namespace sim_mob {

//Forward declaration
class DriverUpdateParams;
class NearestVehicle;
class Driver;


/*
 * \file CarFollowModel.hpp
 *
 * \author Wang Xinyuan
 * \author Li Zhemin
 * \author Seth N. Hetu
 */

//Abstract class which describes car following.
class CarFollowModel {
public:
	//Allow propagation of delete
	virtual ~CarFollowModel() {}

	virtual double makeAcceleratingDecision(sim_mob::DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed) = 0;  ///<Decide acceleration

public:
	string modelName;
	double maxAcceleration;
	/// grade factor
	double accGradeFactor;
	double normalDeceleration;
	double maxDeceleration;

	double hBufferUpper;
	double hBufferLower;
};

/**
 *
 * Simple version of the car following model
 * The purpose of this model is to demonstrate a very simple (yet reasonably accurate) model
 * which generates somewhat plausible visuals. This model should NOT be considered valid, but
 * it can be used for demonstrations and for learning how to write your own *Model subclasses.
 *
 * \author Seth N. Hetu
 */
class SimpleCarFollowModel : public CarFollowModel {
public:
	///Decide acceleration. Simply attempts to reach the target speed.
	virtual double makeAcceleratingDecision(sim_mob::DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed) = 0;
};

//MITSIM version of car following model
class MITSIM_CF_Model : public CarFollowModel {
public:
	MITSIM_CF_Model();
	void initParam();
	/** \brief make index base on speed scaler
	 *  \param speedScalerStr speed scaler
	 *  \param cstr index value
	 *  \param idx  index container
	 *  \param upperBound store upper bound of index
	 **/
	void makeSpeedIndex(Vehicle::VEHICLE_TYPE vhType,
						string& speedScalerStr,
						string& cstr,
						map< Vehicle::VEHICLE_TYPE,map<int,double> >& idx,
						int& upperBound);
	/** \brief create scale index base on string data ,like "0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5"
	 *  \param s data string
	 *  \param c container to store data
	 **/
	void makeScaleIdx(string& s,vector<double>& c);
	double getMaxAcceleration(sim_mob::DriverUpdateParams& p,Vehicle::VEHICLE_TYPE vhType=Vehicle::CAR);
	double getNormalDeceleration(sim_mob::DriverUpdateParams& p,Vehicle::VEHICLE_TYPE vhType=Vehicle::CAR);
	double getMaxDeceleration(sim_mob::DriverUpdateParams& p,Vehicle::VEHICLE_TYPE vhType=Vehicle::CAR);
	/** \brief get max acc scaler
	 *  \return scaler value
	 **/
	double getMaxAccScale();
	double getNormalDecScale();
	double getMaxDecScale();
	virtual double makeAcceleratingDecision(sim_mob::DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed);

private:
	double carFollowingRate(sim_mob::DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed,NearestVehicle& nv);
	double calcSignalRate(sim_mob::DriverUpdateParams& p);
	double calcYieldingRate(sim_mob::DriverUpdateParams& p,double targetSpeed, double maxLaneSpeed);
	double waitExitLaneRate(sim_mob::DriverUpdateParams& p);
	double calcForwardRate(sim_mob::DriverUpdateParams& p);
	double calcBackwardRate(sim_mob::DriverUpdateParams& p);
	double calcAdjacentRate(sim_mob::DriverUpdateParams& p);

	/** \brief return the acc to a target speed within a specific distance
	 *  \param p vehicle state value
	 *  \param s distance (meter)
	 *  \param v velocity (m/s)
	 *  \return acceleration (m/s^2)
	 **/
	double brakeToTargetSpeed(sim_mob::DriverUpdateParams& p,double s,double v);
	double brakeToStop(DriverUpdateParams& p, double dis);
	double accOfEmergencyDecelerating(sim_mob::DriverUpdateParams& p);  ///<when headway < lower threshold, use this function
	double accOfCarFollowing(sim_mob::DriverUpdateParams& p);  ///<when lower threshold < headway < upper threshold, use this function
	double accOfFreeFlowing(sim_mob::DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed);  ///<when upper threshold < headway, use this funcion
	double accOfMixOfCFandFF(sim_mob::DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed); 	///<mix of car following and free flowing
	void distanceToNormalStop(sim_mob::DriverUpdateParams& p);

private:
	/// key=vehicle type
	/// submap key=speed, value=max acc
	map< Vehicle::VEHICLE_TYPE,map<int,double> > maxAccIndex;
	int maxAccUpperBound;
	vector<double> maxAccScale;

	map< Vehicle::VEHICLE_TYPE,map<int,double> > normalDecelerationIndex;
	int normalDecelerationUpperBound;
	vector<double> normalDecelerationScale;

	map< Vehicle::VEHICLE_TYPE,map<int,double> > maxDecelerationIndex;
	int maxDecelerationUpperBound;
	vector<double> maxDecelerationScale;
};


}
