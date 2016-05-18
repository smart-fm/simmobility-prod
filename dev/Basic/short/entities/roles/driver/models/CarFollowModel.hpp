//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/random.hpp>
#include <map>
#include <set>
#include <string>

#include "conf/params/ParameterManager.hpp"
#include "entities/models/Constants.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/driver/DriverPathMover.hpp"
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

	/**Split delimiter in the driver parameters XML file*/
	string splitDelimiter;
	
protected:
	/**The pointer to the driver path mover object*/
	DriverPathMover *fwdDriverMovement;

	/**
	 * Calculates the step size of update state variables
	 *
	 * @param params
	 *
	 * @return
	 */
	double calcNextStepSize(DriverUpdateParams &params);

public:
	CarFollowingModel(DriverPathMover *pathMover) : fwdDriverMovement(pathMover)
	{
	}
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
	/**Represents the container to store the normal distribution*/
	struct UpdateStepSizeParam
	{
		double mean;
		double stdev;
		double lower;
		double upper;
		double perception; //percentage of total reaction time
	};

	/**Represents the container to store the car following model parameters*/
	struct CarFollowingParams
	{
		double alpha;
		double beta;
		double gama;
		double lambda;
		double rho;
		double stddev;
	};

	/**The upper bound on the maximum acceleration*/
	int maxAccUpperBound;

	/**The upper bound on the normal deceleration*/
	int normalDecelerationUpperBound;

	/**The upper bound on the maximum deceleration*/
	int maxDecelerationUpperBound;

	/**This parameter is used to compute the desired speed based on the posted speed limit*/
	double speedFactor;

	/**The visibility distance of the traffic signal*/
	double signalVisibilityDist;

	/**The maximum value of headway a driver can have to pass a yellow light.*/
	double maxYellowLightHeadway;

	/**The minimum speed for approaching a yellow light*/
	double minYellowLightSpeed;

	/**The road slope*/
	double allGrades;

	/**
	 * This is the minimum space headway between the lead and following vehicles for which the following
	 * vehicle must apply some acceleration (or deceleration)
	 */
	double minResponseDistance;

	/**Upper bound for the headway buffer*/
	double hBufferUpper;

	/**Lower bound for the headway buffer*/
	double hBufferLower;

	/**Defines how close the lead vehicle needs to be in order for the following vehicle to brake (in meter)*/
	double visibilityDistance;

	/**Random number generator for calculating update step sizes*/
	boost::mt19937 updateSizeRNG;

	/**The car following parameters*/
	CarFollowingParams CF_parameters[2];
	
	/**Merging parameters*/
	vector<double> mergingParams;

	/**Update step size while deceleration*/
	UpdateStepSizeParam decUpdateStepSize;

	/**Update step size while acceleration*/
	UpdateStepSizeParam accUpdateStepSize;

	/**Update step size when the speed is constant*/
	UpdateStepSizeParam uniformSpeedUpdateStepSize;

	/**Update step size when the vehicle has stopped*/
	UpdateStepSizeParam stoppedUpdateStepSize;

	/**Parameters to calculate the target gap acceleration*/
	vector<double> targetGapAccParm;

	/**The maximum acceleration scale*/
	vector<double> maxAccelerationScale;

	/**The normal deceleration scale*/
	vector<double> normalDecelerationScale;

	/**The maximum deceleration scale*/
	vector<double> maxDecelerationScale;

	/**The speed limit add-on distribution*/
	vector<double> speedLimitAddon;

	/**The car following acceleration add-on distribution*/
	vector<double> accelerationAddon;

	/**The car following deceleration add-on distribution*/
	vector<double> decelerationAddon;

	/**The distribution for the headway upperbound*/
	vector<double> hBufferUpperScale;

	/**The maximum acceleration indices. Key: Vehicle type, Value: Map with Key: Speed, Value: Max. Acceleration*/
	map< VehicleBase::VehicleType, map<int, double> > maxAccelerationIndex;

	/**The normal deceleration indices. Key: Vehicle type, Value: Map with Key: Speed, Value: Normal deceleration*/
	map< VehicleBase::VehicleType, map<int, double> > normalDecelerationIndex;

	/**The maximum deceleration indices. Key: Vehicle type, Value: Map with Key: Speed, Value: Normal deceleration*/
	map< VehicleBase::VehicleType, map<int, double> > maxDecelerationIndex;

	/**
	 * Reads the car following model parameters from the driver parameters XML file
	 *
	 * @param params the drivers parameters
	 */
	void readDriverParameters(DriverUpdateParams &params);

	/**
	 * Creates the car following parameters from the given parameter values in string format
     *
	 * @param strParams the parameter string
     * @param cfParams the car following parameters
     */
	void createCF_Params(string &strParams, CarFollowingParams &cfParams);

	/**
	 * Creates the speed indices based on the speed scalars
	 *
     * @param vehicleType vehicle type
     * @param speedScalerStr speed scalar
     * @param cstr the index value
     * @param idx the container holding the indices
     * @param upperBound the upper bound of the index
     */
	void createSpeedIndices(VehicleBase::VehicleType vehicleType, string &speedScalerStr, string &cstr, map< VehicleBase::VehicleType, map<int, double> > &idx,
							int &upperBound);

	/**
	 * Creates the scale indices based on the string data
     *
	 * @param data the data in the format "0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5"
     * @param container the container that stores the created indices
     */
	void createScaleIndices(string &data, vector<double> &container);

	/**
	 * Creates the update step size parameters from the given parameter values in string format
     * @param strParams the parameter string
     * @param stepSizeParams the container for the update step size parameters
     */
	void createUpdateSizeParams(string &strParams, UpdateStepSizeParam &stepSizeParams);

	/**
	 * Builds and gets a sample from the normal distribution created from the given parameters
     *
	 * @param stepSizeParams the step size parameters from which the distribution is to be created and sampled
     *
	 * @return sampled value from the distribution
     */
	double sampleFromNormalDistribution(UpdateStepSizeParam &stepSizeParams);

	/**
	 * Returns the maximum acceleration for the given vehicle type
	 *
     * @param params the driver parameters
     * @param vhType the vehicle type
	 *
     * @return max acceleration of the given vehicle type
     */
	double getMaxAcceleration(DriverUpdateParams &params, VehicleBase::VehicleType vhType = VehicleBase::CAR);

	/**
	 * Returns the normal deceleration for the given vehicle type
	 *
     * @param params the driver parameters
     * @param vhType the vehicle type
	 *
     * @return normal deceleration of the given vehicle type
     */
	double getNormalDeceleration(DriverUpdateParams &params, VehicleBase::VehicleType vhType = VehicleBase::CAR);

	/**
	 * Returns the maximum deceleration for the given vehicle type
     * @param params the driver parameters
     * @param vhType the vehicle type
	 *
     * @return the maximum deceleration for the given vehicle type
     */
	double getMaxDeceleration(DriverUpdateParams &params, VehicleBase::VehicleType vhType = VehicleBase::CAR);

	/**
	 * Calculates the maximum acceleration scalar
	 *
     * @return maximum acceleration scalar
     */
	double getMaxAccScalar();

	/**
	 * Calculates the normal deceleration scalar
     *
	 * @return normal deceleration scalar
     */
	double getNormalDecScalar();

	/**
	 * Calculates the maximum deceleration scalar
	 *
     * @return maximum deceleration scalar
     */
	double getMaxDecScalar();

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
	double calcMergingAcc(DriverUpdateParams &params);

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

	/**
	 * Calculates the acceleration when the car needs to stop for a red light or needs to start moving for a
	 * green light
	 *
     * @param params the driver parameters
	 *
     * @return acceleration
     */
	double calcTrafficSignalAcc(DriverUpdateParams &params);

	/**
	 * Calculates the acceleration when performing a courtesy yielding
     * @param params the driver parameters
     * @return acceleration
     */
	double calcYieldingAcc(DriverUpdateParams &params);

	/**
	 * Calculate the maximum acceleration for creating a gap to the nearest vehicle
     *
	 * @param params the driver parameters
     * @param nearestVeh the vehicle to which we are trying to open a gap
     * @param gap the gap distance required
     * 
	 * @return acceleration
     */
	double calcAccToCreateGap(DriverUpdateParams &params, NearestVehicle &nearestVeh, float gap);
	
	/**
	 * Calculates the acceleration while exiting a specific lane. For e.g: If a vehicle is not in the correct lane
	 * close to the end of a link, it may decelerate to a stop and wait for lane changing.
     * 
	 * @param params
     * 
	 * @return acceleration
     */
	double calcWaitForLaneExitAcc(DriverUpdateParams &params);

	/**
	 * Calculates the acceleration rate when current lane is incorrect and the distance is close to the
	 * an incurrent lane of the segment
     *
	 * @param params driver parameters
     *
	 * @return acceleration
     */
	double calcWaitForAllowedLaneAcc(DriverUpdateParams& p);

	/**
	 * Calculates the acceleration required to reach the forward gap
	 *
     * @param params driver parameters
     * @return acceleration
     */
	double calcForwardGapAcc(DriverUpdateParams &params);

	/**
	 * Calculates the desired speed
     * @param params driver parameters
     * @return desired speed (m/s)
     */
	double calcDesiredSpeed(DriverUpdateParams &params);

	/**
	 * Calculates the acceleration to reach the backward gap
     *
	 * @param params driver parameters
     *
	 * @return acceleration
     */
	double calcBackwardGapAcc(DriverUpdateParams &params);

	/**
	 * Calculates the acceleration to reach the adjacent gap
     * @param params driver parameters
     * @return acceleration
     */
	double calcAdjacentGapRate(DriverUpdateParams &params);

	/**
	 * Calculates the acceleration required to stop at a stopping point
     * 
	 * @param params
     * 
	 * @return acceleration
     */
	double calcAccForStoppingPoint(DriverUpdateParams &params);

	/**
	 * Calculates acceleration required to reach the desired speed
     * 
	 * @param params driver parameters
     * 
	 * @return acceleration
     */
	double calcDesiredSpeedAcc(DriverUpdateParams &params);

	/**
	 * Calculates the acceleration / deceleration required to speed up / slow down to the target speed
	 * with in the given distance
	 *
     * @param params driver parameters
     * @param distance distance within which the target speed is to be achieved
     * @param velocity the target speed
	 *
     * @return acceleration
     */
	double calcTargetSpeedAcc(DriverUpdateParams &params, double distance, double velocity);

	/**
	 * Calculates the acceleration rate required to accelerate / decelerate from current speed to a full
	 * stop within a given distance
     *
	 * @param params driver parameters
     * @param distance distance within which we need to stop
     *
	 * @return acceleration
     */
	double calcBrakeToStopAcc(DriverUpdateParams &params, double distance);

	/**
	 * Calculates the acceleration required to slow down in an emergency
	 * NOTE: when headway \< lower threshold, use this function
	 *
     * @param params driver parameters
	 *
     * @return acceleration
     */
	double calcEmergencyDeceleration(DriverUpdateParams &params);

	/**
	 * Calculates the acceleration required to follow another car
	 * NOTE: when lower threshold \< headway \< upper threshold, use this function
     * 
	 * @param params driver parameters
     * 
	 * @return acceleration
     */
	double calcAccOfCarFollowing(DriverUpdateParams &params);

	/**
	 * Calculates the acceleration when the traffic is freely flowing
	 *
	 * @param params driver parameters
	 * @param targetSpeed the target speed
	 * @param maxLaneSpeed the speed limit of the current lane
	 *
	 * @return acceleration
	 */
	double calcFreeFlowingAcc(DriverUpdateParams &params, double targetSpeed);

	/**
	 * Calculates either the car following acceleration or the free flow acceleration depending on the gap between
	 * the vehicles
	 *
     * @param params driver parameters
     * @param targetSpeed the desired speed
	 *
     * @return acceleration
     */
	double accOfMixOfCFandFF(DriverUpdateParams &params, double targetSpeed);

	/**
	 * Calculates the distance required for the vehicle to arrive to a stop under normal circumstances
     * 
	 * @param params driver parameters
     */
	void calcDistanceForNormalStop(DriverUpdateParams &params);

	/**
	 * Updates the variables that depend on the vehicle type and speed (i.e. the current state)
	 *
     * @param params driver parameters driver parameters
     */
	void calcStateBasedVariables(DriverUpdateParams& p);

	/**
	 * Calculates the step sizes for making car-following decisions
     */
	void calcUpdateStepSizes();

	/**
	 * Calculates the upper bound of the headway buffer
	 *
     * @return upper bound of the headway buffer
     */
	double getH_BufferUpperBound();

	/**
	 * Calculates a headway buffer for a vehicle, which is a behavioural parameter that describes the aggressiveness
	 * of a driver for accepting a headway gap in lane changing, merging, and car-following.
	 *
     * @return headway (seconds)
     */
	double getHeadwayBuffer();

	/**
	 * Calculates the add on for calculating the desired speed
	 *
     * @return add on value (m/s)
     */
	double getSpeedLimitAddon();

	/**
	 * Calculates the add on for the acceleration
	 *
     * @return add on value (m/s^2)
     */
	double getAccelerationAddon();

	/**
	 * Calculates the add on for the deceleration
	 *
     * @return add on value (m/s^2)
     */
	double getDecelerationAddon();

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
	MITSIM_CF_Model(DriverUpdateParams &params, DriverPathMover *pathMover);
	~MITSIM_CF_Model();

	/**
	 * Calculates the acceleration rate based on interaction with other vehicles. The function returns a the
	 * most restrictive acceleration (deceleration if negative) rate among the rates given by several constraints.
	 *
	 * Car following algorithm is evaluated every stepSize seconds, or whenever some special event has set
	 * reactionTimeCounter of this vehicle to 0. After each evaluation, we set the countdown clock reactionTimeCounter
	 * back to nextStepSize().
	 * 
     * @param params driver parameters
     * 
	 * @return most restrictive acceleration
     */
	virtual double makeAcceleratingDecision(DriverUpdateParams &params);

	double getHBufferUpper()
	{
		return hBufferUpper;
	}
};
}
