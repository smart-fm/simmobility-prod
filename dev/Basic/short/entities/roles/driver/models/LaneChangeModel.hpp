//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <set>

#include "conf/params/ParameterManager.hpp"
#include "geospatial/network/Lane.hpp"
#include "entities/models/Constants.h"
#include "entities/roles/driver/DriverPathMover.hpp"
#include "entities/roles/driver/DriverUpdateParams.hpp"

namespace sim_mob
{
class DriverUpdateParams;
class NearestVehicle;

/**
 * Defines a base template for the LaneChangingModel
 */
class LaneChangingModel
{
protected:
	/**The model name in the "parameters" tag of the XML file*/
	string modelName;

	/**The delimiter to split the parameters specified in the XML file*/
	string splitDelimiter;

	/**The pointer to the driver path mover object*/
	DriverPathMover *fwdDriverMovement;

public:
	LaneChangingModel(DriverPathMover *pathMover) : fwdDriverMovement(pathMover)
	{
	}
	virtual ~LaneChangingModel()
	{
	}

	virtual LaneChangeTo makeLaneChangingDecision(DriverUpdateParams &params) = 0;

	virtual double executeLaneChanging(DriverUpdateParams &params) = 0;

	virtual void chooseTargetGap(DriverUpdateParams &params) = 0;

	virtual double calculateLateralVelocity(LaneChangeTo &laneChangeTo) = 0;

	virtual int checkForEventsAhead(DriverUpdateParams &params) = 0;

	virtual LaneChangeTo checkMandatoryEventLC(DriverUpdateParams &params) = 0;
};

class MITSIM_LC_Model : public LaneChangingModel
{
private:

	/**
	 * Holds the mandatory lane changing parameters
	 */
	struct MandatoryLaneChangeParams
	{
		//In metre
		double lowbound;

		//In metre
		double delta;

		//In sec
		double lane_mintime;
	};

	/**Minimum time require in lane before doing a lane change in the same direction as the previous lane change*/
	double minTimeInLaneSameDir;

	/**Minimum time require in lane before doing a lane change in a direction different to the previous lane change*/
	double minTimeInLaneDiffDir;

	/**Max distance for nosing*/
	double lcMaxNosingDis;

	/**This parameter is used to change drivers reaction time when in a nosing behaviour*/
	double CF_CRITICAL_TIMER_RATIO;

	/**The maximum time a vehicle is stuck while attempting to nose*/
	double lcMaxStuckTime;

	/**Minimum speed to consider for a moving vehicle*/
	double minSpeed;

	/**The look ahead distance*/
	double lookAheadDistance;

	/**Specifies the time horizon (in second) for constant acceleration while nosing*/
	float lcNosingConstStateTime;

	/**Holds the parameters required to calculate critical gap*/
	vector<double> criticalGapParams;

	/**Holds the nosing parameters*/
	vector<double> kaziNosingParams;

	/**Holds the probabilities of yielding to other vehicles*/
	vector<double> lcYieldingProb;

	/**Holds the lane utility model. This model describes drivers’ choice of lane they would want to travel in*/
	vector<double> laneUtilityParams;

	/**Holds the parameters the define how a gap is chosen*/
	std::vector< std::vector<double> > GAP_PARAM;

	/**Holds the mandatory lane changing parameters*/
	MandatoryLaneChangeParams MLC_PARAMETERS;

	/**
	 * Reads the lane changing model parameters from the driver parameters XML file
	 *
	 * @param params the drivers parameters
	 */
	void readDriverParameters(DriverUpdateParams &params);

	/**
	 * Helper function to parse parameters read in string format and store them
	 *
	 * @param str parameters read from driver parameters XML
	 */
	void makeCriticalGapParams(std::string &str);

	/**
	 * Helper function to parse parameters read in string format and store them
	 *
	 * @param params driver parameters
	 * @param str parameters read from driver parameters XML
	 */
	void makeNosingParams(DriverUpdateParams &params, string &str);

	/**
	 * Helper function to parse parameters read in string format and store them
	 *
	 * @param str parameters read from driver parameters XML
	 */
	void makeLC_YieldingProbabilities(string &str);

	/**
	 * Helper function to parse parameters read in string format and store them
	 *
	 * @param str parameters read from driver parameters XML
	 */
	void makeLaneUtilityParams(std::string &str);

	/**
	 * Helper function to parse parameters read in string format and store them
	 *
	 * @param str parameters read from driver parameters XML
	 */
	void makeMCLParam(std::string &str);

	/**
	 * Helper function to parse parameters read in string format and store them
	 *
	 * @param params driver parameters
	 * @param strMatrix parameters read from driver parameters XML
	 */
	void makeCriticalGapParams(DriverUpdateParams &params, std::vector<std::string> &strMatrix);

	/**
	 * Helper function to parse parameters read in string format and store them
	 *
	 * @param strMatrix parameters read from driver parameters XML
	 */
	void makeTargetGapPram(std::vector<std::string> &strMatrix);

	/**
	 * Helper function to parse parameters read in string format and store them
	 *
	 * @param str parameters read from driver parameters XML
	 */
	void makeKaziNosingParams(string &str);

	/**
	 * Calculates target gap utility
	 *
	 * @param params the driver parameters
	 * @param n
	 * @param effectiveGap the gap distance
	 * @param distToGap distance to the gap
	 * @param gapSpeed relative speed
	 * @param remainderGap
	 *
	 * @return
	 */
	double gapExpOfUtility(DriverUpdateParams &params, int n, float effectiveGap, float distToGap, float gapSpeed, float remainderGap);

	/**
	 * This method determines the lane changing direction that a vehicle will try if it is constrained by an event.
	 * 
	 * @param params driver parameters
	 *
	 * @return lane change to direction
	 */
	virtual LaneChangeTo checkMandatoryEventLC(DriverUpdateParams &params);

	/**
	 * This function returns the lane change direction constrained by a look-ahead distance
	 *
	 * @param params driver parameters
	 *
	 * @return lance changing direction
	 */
	LaneChangeTo checkForLC_WithLookAhead(DriverUpdateParams &params);

	/**
	 * This method checks for events that will override the lookahead, like incidents, lane drops
	 *
	 * @param params driver parameters
	 *
	 * @return non-zero value if an event ahead requires a lane change
	 */
	virtual int checkForEventsAhead(DriverUpdateParams &params);

	/**
	 * Sets the dis2stop value if there is an incident ahead that requires a lane change
	 * @param params driver parameters
	 * @return -1 if mandatory lane change required, 0 otherwise
	 */
	int isIncidentAhead(DriverUpdateParams &params);

	/**
	 * Checks if the current lane is connected to the next way-point and sets the dis2stop.
	 * This method checks for connections to the next segment or the turning group, if the next way-point is a turning group
	 *
	 * @param params drivers parameters
	 * @param targetLanes target lanes, if lane change is required
	 *
	 * @return -1 if mandatory lane change, 0 otherwise
	 */
	int isLaneConnectedToNextWayPt(DriverUpdateParams &params, set<const Lane *> &targetLanes);

	/**
	 * Checks if the current lane is connected to the next link and sets the dis2stop.
	 * This method checks for connections to the turning group at the end of the current link
	 *
	 * @param params drivers parameters
	 * @param targetLanes target lanes, if lane change is required
	 *
	 * @return -1 if mandatory lane change, 0 otherwise
	 */
	int isLaneConnectedToNextLink(DriverUpdateParams &params, set<const Lane *> &targetLanes);

	/**
	 * Check if there is a stop point ahead. If so, check if we need to do a lane change towards the road side
	 * 
	 * @param params driver parameters
	 * @param targetLanes the lanes connected to the stop point
	 *
	 * @return -1 if lance change is require, 0 otherwise
	 */
	int isLaneConnectedToStopPoint(DriverUpdateParams &params, set<const Lane *> &targetLanes);
	
	/**
	 * Gets the lanes connected to the segment within the look ahead distance
	 * 
	 * @param params driver parameters
     * @param lookAheadDist look ahead distance
     * @param lanePool stores the result
     */
	void getConnectedLanesInLookAheadDistance(DriverUpdateParams &params, double lookAheadDist, std::vector<Lane *> &lanePool);

	/**
	 * Calculates the utility of the left lane when looking ahead for lane change
	 *
	 * @param params driver parameters
	 * @param noOfChanges number of lane changes
	 * @param lcDistance distance before which lane change must be done
	 * 
	 * @return utility of the left lane
	 */
	double lcUtilityLookAheadLeft(DriverUpdateParams &params, int noOfChanges, float lcDistance);

	/**
	 * Calculates the utility of the right lane when looking ahead for lane change
	 *
	 * @param params driver parameters
	 * @param noOfChanges number of lane changes
	 * @param lcDistance distance before which lane change must be done
	 *
	 * @return utility of the right lane
	 */
	double lcUtilityLookAheadRight(DriverUpdateParams &params, int noOfChanges, float lcDistance);

	/**
	 * Calculates the utility of the current lane when looking ahead for lane change
	 *
	 * @param params driver parameters
	 * @param noOfChanges number of lane changes
	 * @param lcDistance distance before which lane change must be done
	 *
	 * @return utility of the current lane
	 */
	double lcUtilityLookAheadCurrent(DriverUpdateParams &params, int noOfChanges, float lcDistance);

	/**
	 * Calculates the utility of the left lane for mandatory lane change
	 *
	 * @param params driver parameters
	 *
	 * @return utility of the left lane
	 */
	double lcUtilityLeft(DriverUpdateParams &params);

	/**
	 * Calculates the utility of the right lane for mandatory lane change
	 *
	 * @param params driver parameters
	 *
	 * @return utility of the right lane
	 */
	double lcUtilityRight(DriverUpdateParams &params);

	/**
	 * Calculates the utility of the current lane for mandatory lane change
	 *
	 * @param params driver parameters
	 *
	 * @return utility of the current lane
	 */
	double lcUtilityCurrent(DriverUpdateParams &params);

	/**
	 * Finds the number of lane changes to end of the link.
	 *
	 * @param params driver parameters
	 * @param currLane the current lane
	 *
	 * @return number of lane changes required
	 */
	int getNumberOfLCToEndOfLink(DriverUpdateParams &params, const Lane *currLane);

	/**
	 * Calculates the critical gap for the given type of gap and the given differences in the speeds
	 *
	 * @param params the driver parameters
	 * @param type the type of gap. 0: Lead gap, 1: Lag gap
	 * @param dv difference the in the speeds of the vehicles
	 *
	 * @return value of critical gap
	 */
	double lcCriticalGap(DriverUpdateParams &params, int type, double dv);

	/**
	 * Calculates the nosing probability
	 * @param distance distance before which nosing is to be completed
	 * @param diffInSpeed the difference in the speeds of the vehicles
	 * @param gap the gap between the vehicles
	 * @param num number of lanes
	 *
	 * @return nosing probability
	 */
	float lcNosingProbability(float distance, float diffInSpeed, float gap, int num);

	/**
	 *
	 * @param params
	 * @param fwdVehicle
	 * @param rearVehicle
	 * @param distToStop
	 * @return 1 if the vehicle can nose in, 0 otherwise
	 */
	int checkNosingFeasibility(DriverUpdateParams &params, const NearestVehicle *fwdVehicle, const NearestVehicle *rearVehicle, double distToStop);

	/**
	 * Checks the connections to the next segment/turning groups and sets or clears the status
	 * STATUS_LEFT_OK, STATUS_RIGHT_OK, STATUS_CURRENT_OK
	 *
	 * @param params driver parameters
	 */
	void setLaneConnectionStatus(DriverUpdateParams &params);

	/**
	 * Calculates the time since the vehicle was tagged
	 *
	 * @param params
	 *
	 * @return time since tagged (in seconds)
	 */
	double timeSinceTagged(DriverUpdateParams &params);

	/**
	 * Mandatory lane change distance for lookahead vehicles
	 * @return lookahead distance (metre)
	 */
	double mlcDistance();

	/**
	 * Uses the Kazi LC Gap Model to calculate the critical gap
	 *
	 * @param params drivers parameters
	 * @param type 0=leading 1=lag + 2=mandatory (mask)
	 * @param distance distance from critical position
	 * @param diffInSpeed difference in speeds between the follower and the leader
	 *
	 * @return the critical gap
	 */
	double calcCriticalGapKaziModel(DriverUpdateParams &params, int type, double distance, double diffInSpeed);

public:
	MITSIM_LC_Model(DriverUpdateParams &params, DriverPathMover *pathMover);
	virtual ~MITSIM_LC_Model();

	/**
	 * This is the lane changing model.
	 * This function sets bits 4-7 of the variable 'status'. The fourth and fifth bit indicate current lane change status,
	 * and the bits should be masked by:
	 *  8=STATUS_RIGHT
	 * 16=STATUS_LEFT
	 * 24=STATUS_CHANGING
	 * This function is invoked when the countdown clock cfTimer is 0.
	 *
	 * @param params the driver parameters
	 *
	 * @return a non-zero value if the vehicle needs a lane change,
	 * and 0 otherwise
	 */
	virtual LaneChangeTo makeLaneChangingDecision(DriverUpdateParams &params);

	/**
	 * Chooses the target gap for lane change. When left and right gap is not possible we choose adjacent/forward/backward gap
	 * and set the status to STATUS_ADJACENT/STATUS_FORWARD/STATUS_BACKWARD
	 *
	 * @param params the driver parameters
	 */
	virtual void chooseTargetGap(DriverUpdateParams &params);

	/**
	 * This method checks whether we can perform lane change (set status STATUS_LC_RIGHT, STATUS_LC_LEFT). If we can't, then
	 * it checks whether we can perform nosing
	 *
	 * @param params driver parameters
	 *
	 * @return 0
	 */
	virtual double executeLaneChanging(DriverUpdateParams &params);

	/**
	 * Checks whether we are ready to perform the next discretionary lane change (DLC)
	 *
	 * @param params the driver parameters
	 * @param mode indicates the lane change direction [1: change to right, 2: change to left]
	 * 
	 * @return 1 if the lane change is possible, 0 otherwise
	 */
	int isReadyForNextDLC(DriverUpdateParams &params, int mode);

	/**
	 * Calculate the lateral velocity for the lane change
	 *
	 * @param change the lane to be changed to
	 *
	 * @return the lateral velocity for the lane change
	 */
	virtual double calculateLateralVelocity(LaneChangeTo &change);
};
}
