//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "conf/params/ParameterManager.hpp"
#include "entities/vehicle/VehicleBase.hpp"
#include "../short/entities/roles/driver/Driver.hpp"
#include "entities/models/Constants.hpp"

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

	virtual double makeAcceleratingDecision(sim_mob::DriverUpdateParams& p) = 0;  ///<Decide acceleration
public:
	string modelName;
//	double maxAcceleration;
	/// grade factor
	double accGradeFactor;
//	double normalDeceleration;
//	double maxDeceleration;

//	double nextStepSize;
	double nextPerceptionSize;

	double minSpeed;

//	double getNextStepSize() { return nextStepSize; }
	/// update step size , dec,acc,uniform speed,stopped vh
	std::vector<double> updateStepSize;
	std::vector<double> perceptionSize;
	/** \brief calculate the step size of update state variables
	 *         the vehicle and type.
	 *  \param p vehicle state value
	 *  \return step size
	 **/
	double calcNextStepSize(DriverUpdateParams& p);
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
	//Simple struct to hold Car Following model parameters
	struct CarFollowParam {
		double alpha;
		double beta;
		double gama;
		double lambda;
		double rho;
		double stddev;
	};

	MITSIM_CF_Model(sim_mob::DriverUpdateParams& p);
	void initParam(sim_mob::DriverUpdateParams& p);
	/** \brief make index base on speed scaler
	 *  \param speedScalerStr speed scaler
	 *  \param cstr index value
	 *  \param idx  index container
	 *  \param upperBound store upper bound of index
	 **/
	void makeSpeedIndex(VehicleBase::VehicleType vhType,
						string& speedScalerStr,
						string& cstr,
						map< VehicleBase::VehicleType,map<int,double> >& idx,
						int& upperBound);
	/** \brief create scale index base on string data ,like "0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5"
	 *  \param s data string
	 *  \param c container to store data
	 **/
	void makeScaleIdx(string& s,vector<double>& c);
	double getMaxAcceleration(sim_mob::DriverUpdateParams& p,VehicleBase::VehicleType vhType=VehicleBase::CAR);
	double getNormalDeceleration(sim_mob::DriverUpdateParams& p,VehicleBase::VehicleType vhType=VehicleBase::CAR);
	double getMaxDeceleration(sim_mob::DriverUpdateParams& p,VehicleBase::VehicleType vhType=VehicleBase::CAR);
	/** \brief get max acc scaler
	 *  \return scaler value
	 **/
	double getMaxAccScale();
	/** \brief normal deceleration scaler
	 *  \return scaler value
	 **/
	double getNormalDecScale();
	/** \brief max deceleration scaler
	 *  \return scaler value
	 **/
	double getMaxDecScale();
	/**
		* The Car-Following model calculates the acceleration rate based on
		* interaction with other vehicles.  The function returns a the
		* most restrictive acceleration (deceleration if negative) rate
		* among the rates given by several constraints.
		*
		* This function updates accRate_ at the end.

		* Car following algorithm is evaluated every CFStepSize seconds,
		* or whenever some special event has set cfTimer of this vehicle
		* to 0. After each evaluation, we set the countdown clock cfTimer
		* back to nextStepSize().
	 **/
	virtual double makeAcceleratingDecision(sim_mob::DriverUpdateParams& p);

private:
	double carFollowingRate(sim_mob::DriverUpdateParams& p,NearestVehicle& nv);
	double calcCarFollowingRate(DriverUpdateParams& p);
	/**
	 *  /brief Check if the vehicle is in merging area
	 */
	int isInMergingArea(DriverUpdateParams& p);
	vector<double> mergingParams;
	double upMergingArea() { return mergingParams[0]; }
	double dnMergingArea() { return mergingParams[1]; }
	int nVehiclesAllowedInMergingArea() {
		return (int) mergingParams[2];
	  }
	float aggresiveRampMergeProb() {
		return mergingParams[3];
	  }

	/**
	 *  /brief Calculate the acceleration rate by merging constraint.
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 */
	double calcMergingRate(sim_mob::DriverUpdateParams& p);
	/**
	 *  /brief Check if this driver considers the gap from an incoming vehciel
	 *   to be an acceptable gap for merging or crossing.
	 *
	 *   It depends on the speed of the coming vehicle and this driver's
	 *   behavior parameters (see headwayBuffer() in <omodels.C>). The
	 *
	 *  \param vh the object vehicle
	 *  \return returns true if the gap is acceptable, or false otherwise.
	 */
	bool isGapAcceptable(sim_mob::DriverUpdateParams& p,NearestVehicle& vh);
	/** \brief this function calculates the acceleration rate when the car is stopped at a traffic light
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 **/
	double calcSignalRate(sim_mob::DriverUpdateParams& p);
	/**
	 *  \brief The function calcYieldingRate calculates the acceleration rate when performing a courtesy yielding.
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 */
	double calcYieldingRate(sim_mob::DriverUpdateParams& p);
	/*
	 *  /brief Calculate the maximum acceleration rate subject to the the gap from the leading vehicle.
	 *
	 */
	double calcCreateGapRate(DriverUpdateParams& p,NearestVehicle& vh,float gap);
	/** \brief this function calculates the acceleration rate before exiting a specific lane
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 **/
	double waitExitLaneRate(sim_mob::DriverUpdateParams& p);
	/** \brief this function calculates the acceleration rate when
	 *   current lane is incorrect and the distance is close to the an incurrent lane of the segment
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 **/
	double waitAllowedLaneRate(sim_mob::DriverUpdateParams& p);

	/**
	 *  \brief The function calcForwardRate calculates the acceleration for the forward gap
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 */
	double calcForwardRate(sim_mob::DriverUpdateParams& p);

	/*
	 *  \brief calculate desired speed
	 *  \param p driver's parameters
	 *  \return speed m/s
	 */
	double calcDesiredSpeed(sim_mob::DriverUpdateParams& p);

	/**
	 *  \brief The function calcBackwardRate calculates the acceleration for the backward gap
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 */
	double calcBackwardRate(sim_mob::DriverUpdateParams& p);
	/**
	 *  \brief The function calcAdjacentRate calculates the acceleration for the adjacent gap
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 */
	double calcAdjacentRate(sim_mob::DriverUpdateParams& p);

	/**
	 *  @brief calculates the acceleration for stop point
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 */
	double calcStopPointRate(sim_mob::DriverUpdateParams& p);

	/** \brief return the acc to a target speed within a specific distance
	 *  \param p vehicle state value
	 *  \param s distance (meter)
	 *  \param v velocity (m/s)
	 *  \return acceleration (m/s^2)
	 **/
	double desiredSpeedRate(sim_mob::DriverUpdateParams& p);
	double brakeToTargetSpeed(sim_mob::DriverUpdateParams& p,double s,double v);
	double brakeToStop(DriverUpdateParams& p, double dis);
	double accOfEmergencyDecelerating(sim_mob::DriverUpdateParams& p);  ///<when headway < lower threshold, use this function
	double accOfCarFollowing(sim_mob::DriverUpdateParams& p);  ///<when lower threshold < headway < upper threshold, use this function
	double accOfFreeFlowing(sim_mob::DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed);  ///<when upper threshold < headway, use this funcion
	double accOfMixOfCFandFF(sim_mob::DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed); 	///<mix of car following and free flowing
	void distanceToNormalStop(sim_mob::DriverUpdateParams& p);
	/** \brief This function update the variables that depend only on the speed of
	 *         the vehicle and type.
	 *  \param p vehicle state value
	 **/
	void calcStateBasedVariables(DriverUpdateParams& p);

	/** \brief Calculate the step sizes for making car-following decisions, load only when init
	 *
	 **/
	void calcUpdateStepSizes();

public:
	// split delimiter in xml param file
	string splitDelimiter;
	/// key=vehicle type
	/// submap key=speed, value=max acc
	map< VehicleBase::VehicleType,map<int,double> > maxAccIndex;
	int maxAccUpperBound;
	vector<double> maxAccScale;

	map< VehicleBase::VehicleType,map<int,double> > normalDecelerationIndex;
	int normalDecelerationUpperBound;
	vector<double> normalDecelerationScale;

	double speedFactor;
	/// store speed limit addon parameter
	vector<double> speedLimitAddon;

	/// driver signal perception distance
	double percepDisM;

	/**
	 *  /brief calculate speed add on
	 *  /return add on value
	 */
	double getSpeedLimitAddon();

	vector<double> accAddon;
	vector<double> declAddon;
	double getAccAddon();
	double getDeclAddon();

	map< VehicleBase::VehicleType,map<int,double> > maxDecelerationIndex;
	int maxDecelerationUpperBound;
	vector<double> maxDecelerationScale;

	// param of calcSignalRate()
	double yellowStopHeadway;
	double minSpeedYellow;

//	/// decision timer (second)
//	double cftimer;

	/// grade is the road slope
	double tmpGrade;


	double minResponseDistance;

	// param of carFollowingRate()
	double hBufferUpper;
	vector<double> hBufferUpperScale;
	/** \brief calculate hBufferUpper value uniform distribution
	 *  \return hBufferUpper
	 **/
	double getBufferUppder();
	double hBufferLower;
	/**
	 * /brief Find a headway buffer for a vehicle.  This is a behavior parameter
	 *        that describe how aggressive for a driver to accept a headway
	 *        gap in lane changing, merging, and car-following.  The value
	*         return by this function will be added to the minimum headway gaps
	*         for the population, which are constants provided in parameter
	*         file.
	*
	* The returned value is in seconds.
	 **/
	double headwayBuffer();

	//Car following parameters
	CarFollowParam CF_parameters[2];
	/** \brief convert string to CarFollowParam
	 *  \param s string data
	 *  \param cfParam CarFollowParam to store converted double value
	 **/
	void makeCFParam(string& s,CarFollowParam& cfParam);

	// target gap parameters
	vector<double> targetGapAccParm;



	/// param of normal distributions
	struct UpdateStepSizeParam
	{
		double mean;
		double stdev;
		double lower;
		double upper;
		double percep; // percentage of total reaction time
	};
	/// deceleration update size
	UpdateStepSizeParam decUpdateStepSize;
	/// acceleration update size
	UpdateStepSizeParam accUpdateStepSize;
	/// uniform Speed update size
	UpdateStepSizeParam uniformSpeedUpdateStepSize;
	/// stopped vehicle update size
	UpdateStepSizeParam stoppedUpdateStepSize;
	/** \brief convert string to CarFollowParam
	 *  \param s string data
	 *  \param cfParam CarFollowParam to store converted double value
	 **/
	void makeUpdateSizeParam(string& s,UpdateStepSizeParam& sParam);
	boost::mt19937 updateSizeRm;
	double makeNormalDist(UpdateStepSizeParam& sp);

	double visibilityDistance;
	double visibility() { return visibilityDistance; }
};


}
