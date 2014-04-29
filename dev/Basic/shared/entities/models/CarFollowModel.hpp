//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "conf/params/ParameterManager.hpp"

namespace sim_mob {

//Forward declaration
class DriverUpdateParams;
class NearestVehicle;


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
	virtual double makeAcceleratingDecision(sim_mob::DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed);

private:
	double carFollowingRate(sim_mob::DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed,NearestVehicle& nv);
	double calcSignalRate(sim_mob::DriverUpdateParams& p);
	double calcYieldingRate(sim_mob::DriverUpdateParams& p,double targetSpeed, double maxLaneSpeed);
	double waitExitLaneRate(sim_mob::DriverUpdateParams& p);
	double calcForwardRate(sim_mob::DriverUpdateParams& p);
	double calcBackwardRate(sim_mob::DriverUpdateParams& p);
	double calcAdjacentRate(sim_mob::DriverUpdateParams& p);
	double brakeToTargetSpeed(sim_mob::DriverUpdateParams& p);  ///<return the acc to a target speed within a specific distance
	double brakeToStop(DriverUpdateParams& p, double dis);
	double accOfEmergencyDecelerating(sim_mob::DriverUpdateParams& p);  ///<when headway < lower threshold, use this function
	double accOfCarFollowing(sim_mob::DriverUpdateParams& p);  ///<when lower threshold < headway < upper threshold, use this function
	double accOfFreeFlowing(sim_mob::DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed);  ///<when upper threshold < headway, use this funcion
	double accOfMixOfCFandFF(sim_mob::DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed); 	///<mix of car following and free flowing
	void distanceToNormalStop(sim_mob::DriverUpdateParams& p);
};


}
