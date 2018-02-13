//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "config/params/ParameterManager.hpp"
#include "entities/roles/driver/Driver.hpp"

using namespace std;

namespace sim_mob
{

class DriverUpdateParams;

class VehicleLoadingModel
{
private:
	/**The model name*/
	string modelName;

	/**Split delimiter in the driver parameters XML file*/
	string splitDelimiter;

	/**Represents the various distance (to leader vehicle) headway thresholds for the initial speed assignment*/
	struct InitialSpeedAssignmentThresholds
	{
		//Lower bound threshold for bounded traffic
		double distanceBoundedLow;

		//Upper bound threshold for bounded traffic
		double distanceBoundedUpper;

		//Threshold for unbounded traffic
		double distanceUnbounded;
	};

	InitialSpeedAssignmentThresholds speedAssignmentThresholds;

	/**
	 * If the given start segment is in the path, the path is updated to start from the given segment
	 * @param path the path
	 * @param segmentId the initial segment provided through the configuration file
	 */
	void setPathStartSegment(vector<WayPoint> &path, const int segmentId) const;

	/**
	 * Gets the lanes that connect the starting link in the path to the next link
	 * @param path the path
	 * @param connectedLanes the lanes that are connected to the next link
	 * @param params the driver update parameters
	 */
	void chooseConnectedLanes(vector<WayPoint> &path, set<const Lane *> &connectedLanes, DriverUpdateParams &params);

	/**
	 * Selects the lane with the most amount of space at the entry point of the lane
	 * @param connectedLanes the lanes that are connected to the next link
	 * @param leadVehicles the map of candidate lane vs the lead vehicles in that lane
	 * @param followerVehicles the map of the candidate lane vs the follower vehicles in that lane
	 * @return the lane with the most amount of space
	 */
	const Lane* chooseLaneWithMostSpace(const set<const Lane *> &connectedLanes,
	                                    unordered_map<const Lane *, NearestVehicle> &leadVehicles,
	                                    unordered_map<const Lane *, NearestVehicle> &followerVehicles);

public:
	VehicleLoadingModel(DriverUpdateParams &params);
	~VehicleLoadingModel();

	/**
	 * Reads the vehicle loading model parameters from the driver parameters XML file
	 *
	 * @param params the drivers parameters
	 */
	void readDriverParameters(DriverUpdateParams &params);

	/**
	 * Creates the distance thresholds for the initial speed assignment parameters
	 * from the given parameter values in string format
     * @param strParams the parameter string
     * @param thresholds the container for the threshold parameters
     */
	void createInitialSpeedAssignmentThresholds(string &strParams, InitialSpeedAssignmentThresholds &thresholds);

	/**
	 * Gets the nearby vehicles and then filters out the lead and follower vehicles in each of the candidate
	 * lanes
	 * @param params driver update parameters
	 * @param candidateLanes the short-listed lanes
	 * @param leadVehicles the map of candidate lane vs the lead vehicles in that lane
	 * @param followerVehicles the map of the candidate lane vs the follower vehicles in that lane
	 */
	void getLeadAndFollowerVehicles(DriverUpdateParams &params, set<const Lane *> &candidateLanes,
	                                unordered_map<const Lane *, NearestVehicle> &leadVehicles,
	                                unordered_map<const Lane *, NearestVehicle> &followerVehicles);

	/**
	 * Sets the starting lane index in the current path and sets the initial speed
	 * @param path the path chosen by the route choice model
	 * @param laneIdx if -1 select lane using model, else this is the lane index provided through the configuration file
	 * @param segmentId initial segment provided through the configuration file, 0 (default) indicates first segment
	 * in path
	 * @param initialSpeed 0 by default, set by using the loading model
	 * @param params the driver update parameters
	 */
	void chooseStartingLaneAndSpeed(vector<WayPoint> &path, int *laneIdx, const int segmentId, int *initialSpeed,
	                                DriverUpdateParams &params);

	InitialSpeedAssignmentThresholds& getInitialSpeedAssignmentThresholds()
	{
		return speedAssignmentThresholds;
	}
};
}