//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/random.hpp>
#include <map>
#include <set>
#include <string>

#include "conf/params/ParameterManager.hpp"
#include "entities/models/Constants.h"
#include "entities/roles/driver/Driver.hpp"
#include "entities/vehicle/VehicleBase.hpp"

using namespace std;

namespace sim_mob
{
class DriverUpdateParams;
class NearestVehicle;
class Driver;

/**Abstract class which describes car following*/
class CarFollowingModel
{	
public:
	/**The model name*/
	string modelName;

	/**The acceleration grade factor*/
	double accGradeFactor;

	/**The next perception size*/
	double nextPerceptionSize;

	/**Minimum speed*/
	double minSpeed;

	/**The update step sizes*/
	std::vector<double> updateStepSize;

	/**The perception sizes*/
	std::vector<double> perceptionSize;

	/**
	 * Calculates the step size of update state variables
     *
	 * @param params
     *
	 * @return
     */
	double calcNextStepSize(DriverUpdateParams &params);

public:
	virtual ~CarFollowingModel()
	{
		updateStepSize.clear();
		perceptionSize.clear();
	}

	/**
	 * Calculates the acceleration rate based on interaction with other vehicles
	 *
     * @param params the update parameters
	 *
     * @return the acceleration
     */
	virtual double makeAcceleratingDecision(DriverUpdateParams &params) = 0;
};

/**
 * MITSIM version of car following model
 */
class MITSIM_CF_Model : public CarFollowingModel
{
private:
	/**Merging parameters*/
	vector<double> mergingParams;

	/**
	 * Calculates the acceleration based on the car-following constraints. 
	 *
	 * CAUTION: The two vehicles concerned in this function may not necessarily be in the same lane or even the
	 * same segment.
	 * A modified GM model is used in this implementation.
	 *
     * @param params the update parameters
     * @param nearestVehicle the nearest vehicle
     *
	 * @return acceleration
     */
	double calcCarFollowingAcc(DriverUpdateParams &params, NearestVehicle &nearestVehicle);

	/**
	 * Calculates the car following accelerations for 2 scenarios:
	 * 1. Vehicle in front is on the same link
	 * 2. Vehicle in front is on the next link (beyond the intersection)
	 * and returns the lower of the two values
	 *
     * @param params the update parameters
	 *
     * @return acceleration
     */
	double calcCarFollowingAcc(DriverUpdateParams &params);

	/**
	 * Calculate the acceleration based on the merging constraints
	 *
     * @param params the update parameters
	 *
     * @return acceleration
     */
	double calcMergingRate(DriverUpdateParams &params);

	/**
	 * Checks if this driver considers the gap from an incoming vehicle to be an acceptable gap for merging or crossing.
	 * It depends on the speed of the coming vehicle and this driver's behaviour parameters
	 *
     * @param params update parameters
     * @param nearestVehicle the incoming vehicle
	 *
     * @return returns true if the gap is acceptable, or false otherwise.
     */
	bool isGapAcceptable(DriverUpdateParams &params, NearestVehicle &nearestVehicle);

	/** \brief this function calculates the acceleration rate when the car is stopped at a traffic light
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 **/
	double calcSignalRate(DriverUpdateParams& p);
	/**
	 *  \brief The function calcYieldingRate calculates the acceleration rate when performing a courtesy yielding.
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 */
	double calcYieldingRate(DriverUpdateParams& p);
	/*
	 *  /brief Calculate the maximum acceleration rate subject to the the gap from the leading vehicle.
	 *
	 */
	double calcCreateGapRate(DriverUpdateParams& p, NearestVehicle& vh, float gap);
	/** \brief this function calculates the acceleration rate before exiting a specific lane
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 **/
	double waitExitLaneRate(DriverUpdateParams& p);
	/** \brief this function calculates the acceleration rate when
	 *   current lane is incorrect and the distance is close to the an incurrent lane of the segment
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 **/
	double waitAllowedLaneRate(DriverUpdateParams& p);

	/**
	 *  \brief The function calcForwardRate calculates the acceleration for the forward gap
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 */
	double calcForwardRate(DriverUpdateParams& p);

	/*
	 *  \brief calculate desired speed
	 *  \param p driver's parameters
	 *  \return speed m/s
	 */
	double calcDesiredSpeed(DriverUpdateParams& p);

	/**
	 *  \brief The function calcBackwardRate calculates the acceleration for the backward gap
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 */
	double calcBackwardRate(DriverUpdateParams& p);
	/**
	 *  \brief The function calcAdjacentRate calculates the acceleration for the adjacent gap
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 */
	double calcAdjacentRate(DriverUpdateParams& p);

	/**
	 *  @brief calculates the acceleration for stop point
	 *  \param p driver's parameters
	 *  \return acceleration rate
	 */
	double calcStopPointRate(DriverUpdateParams& p);

	/** \brief return the acc to a target speed within a specific distance
	 *  \param p vehicle state value
	 *  \param s distance (meter)
	 *  \param v velocity (m/s)
	 *  \return acceleration (m/s^2)
	 **/
	double desiredSpeedRate(DriverUpdateParams& p);
	double brakeToTargetSpeed(DriverUpdateParams& p, double s, double v);
	double brakeToStop(DriverUpdateParams& p, double dis);
	double accOfEmergencyDecelerating(DriverUpdateParams& p); ///<when headway < lower threshold, use this function
	double accOfCarFollowing(DriverUpdateParams& p); ///<when lower threshold < headway < upper threshold, use this function

	/**
	 * Calculates the free flowing acceleration
     * @param params
     * @param targetSpeed
     * @param maxLaneSpeed
	 *
     * @return acceleration
     */
	double calcFreeFlowingAcc(DriverUpdateParams &params, double targetSpeed, double maxLaneSpeed);

	double accOfMixOfCFandFF(DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed); ///<mix of car following and free flowing
	void distanceToNormalStop(DriverUpdateParams& p);
	/** \brief This function update the variables that depend only on the speed of
	 *         the vehicle and type.
	 *  \param p vehicle state value
	 **/
	void calcStateBasedVariables(DriverUpdateParams& p);

	/** \brief Calculate the step sizes for making car-following decisions, load only when init
	 *
	 **/
	void calcUpdateStepSizes();

	double upMergingArea()
	{
		return mergingParams[0];
	}

	double dnMergingArea()
	{
		return mergingParams[1];
	}

	int nVehiclesAllowedInMergingArea()
	{
		return (int) mergingParams[2];
	}

	float aggresiveRampMergeProb()
	{
		return mergingParams[3];
	}
	
public:
	/**Holds the car following model parameters*/
	struct CarFollowingParams
	{
		double alpha;
		double beta;
		double gama;
		double lambda;
		double rho;
		double stddev;
	};

	MITSIM_CF_Model(DriverUpdateParams& p);
	void initParam(DriverUpdateParams& p);
	/** \brief make index base on speed scaler
	 *  \param speedScalerStr speed scaler
	 *  \param cstr index value
	 *  \param idx  index container
	 *  \param upperBound store upper bound of index
	 **/
	void makeSpeedIndex(VehicleBase::VehicleType vhType,
						string& speedScalerStr,
						string& cstr,
						map< VehicleBase::VehicleType, map<int, double> >& idx,
						int& upperBound);
	/** \brief create scale index base on string data ,like "0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5"
	 *  \param s data string
	 *  \param c container to store data
	 **/
	void makeScaleIdx(string& s, vector<double>& c);
	double getMaxAcceleration(DriverUpdateParams& p, VehicleBase::VehicleType vhType = VehicleBase::CAR);
	double getNormalDeceleration(DriverUpdateParams& p, VehicleBase::VehicleType vhType = VehicleBase::CAR);
	double getMaxDeceleration(DriverUpdateParams& p, VehicleBase::VehicleType vhType = VehicleBase::CAR);
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
	virtual double makeAcceleratingDecision(DriverUpdateParams& p);

public:
	// split delimiter in xml param file
	string splitDelimiter;
	/// key=vehicle type
	/// submap key=speed, value=max acc
	map< VehicleBase::VehicleType, map<int, double> > maxAccIndex;
	int maxAccUpperBound;
	vector<double> maxAccScale;

	map< VehicleBase::VehicleType, map<int, double> > normalDecelerationIndex;
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

	map< VehicleBase::VehicleType, map<int, double> > maxDecelerationIndex;
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
	CarFollowingParams CF_parameters[2];
	/** \brief convert string to CarFollowParam
	 *  \param s string data
	 *  \param cfParam CarFollowParam to store converted double value
	 **/
	void makeCFParam(string& s, CarFollowingParams& cfParam);

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
	void makeUpdateSizeParam(string& s, UpdateStepSizeParam& sParam);
	boost::mt19937 updateSizeRm;
	double makeNormalDist(UpdateStepSizeParam& sp);

	double visibilityDistance;

	double visibility()
	{
		return visibilityDistance;
	}
};
}
