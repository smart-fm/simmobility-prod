//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * DriverFacets.hpp
 *
 *  Created on: May 15th, 2013
 *      Author: Yao Jin
 */

#pragma once
#include <string>
#include "conf/settings/DisableMPI.h"
#include "Driver.hpp"
#include "DriverPathMover.hpp"
#include "DriverUpdateParams.hpp"
#include "entities/amodController/AMODController.hpp"
#include "entities/IncidentStatus.hpp"
#include "entities/roles/driver/models/CarFollowModel.hpp"
#include "entities/roles/driver/models/LaneChangeModel.hpp"
#include "entities/roles/Role.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "geospatial/Incident.hpp"
#include "IncidentPerformer.hpp"
#include "util/OneTimeFlag.hpp"

namespace
{
/**Constant distance value used for looking ahead (metre)*/
const static int distanceInFront = 50;

/**Constant distance value used for looking behind  (metre)*/
const static int distanceBehind = 30;

/**Constant distance value used for looking ahead (metre)*/
const static int maxVisibleDis = 100;
}

namespace sim_mob
{
class CarFollowingModel;

class DriverBehavior : public BehaviorFacet
{
protected:
	Driver *parentDriver;

public:
	explicit DriverBehavior() :
	BehaviorFacet(), parentDriver(NULL)
	{
	}
	
	virtual ~DriverBehavior()
	{
	}

	virtual void frame_init()
	{
		throw std::runtime_error("Not implemented: DriverBehavior::frame_init()");
	}
	
	virtual void frame_tick()
	{
		throw std::runtime_error("Not implemented: DriverBehavior::frame_tick()");
	}
	
	virtual std::string frame_tick_output()
	{
		throw std::runtime_error("Not implemented: DriverBehavior::frame_tick_output()");
	}

	Driver* getParentDriver() const
	{
		return parentDriver;
	}

	void setParentDriver(Driver *parentDriver)
	{
		if (!parentDriver)
		{
			throw std::runtime_error("parentDriver cannot be NULL");
		}
		safe_delete_item(this->parentDriver);
		this->parentDriver = parentDriver;
	}
} ;

class DriverMovement : public MovementFacet
{
private:
	/**The driver whose movement is being simulated by the DriverMovement object*/
	Driver *parentDriver;

	/**The traffic signal at the approaching intersection. If the intersection is un-signalised, this will be null*/
	const Signal *trafficSignal;

	/**
	 * The index of the target lane. The target lane is the lane we want to be in after crossing the
	 * intersection (In short, this is the index of the lane pointed to by nextLaneInNextLink)
	 */
	unsigned int targetLaneIndex;

	/**Map of road segment vs the aggregate vehicle count over the collection interval*/
	static map<const RoadSegment *, unsigned long> rdSegDensityMap;

	/**Mutex to lock the density map*/
	static boost::mutex densityUpdateMutex;

	/**
	 * Sets the distance and the driver of the NearestVehicle object given. The distance is the distance between
	 * the current driver and the other driver
	 *
     * @param neasrestVeh The object representing the nearest vehicle
     * @param distance The distance between the current vehicle and the nearest vehicle
     * @param otherDriver The driver of the nearest vehicle
     */
	void setNearestVehicle(NearestVehicle &nearestVeh, double distance, const Driver *otherDriver);

	/**
	 * Updates the perceived values
	 *
     * @param params The parameters to be updated
     */
	void perceiveParameters(DriverUpdateParams &params);

	/**
	 * Updates the information that is sensed. Such as, the positions of nearby vehicles, the traffic signal
     *
	 * @return true, unless the driver has completed the trip
     */
	bool updateSensors();

	/**
	 * Moves the vehicle forward according to the accelerations and velocities calculated by the various driver models
     *
	 * @return true, unless the driver has completed the trip
     */
	bool updateMovement();

	/**
	 * Deals with the effect of moving forward - Chooses next lane in next link, checks if we're approaching an
	 * intersection, calculates the intersection trajectory if we've moved into an intersection
     *
	 * @return true, unless the driver has completed the trip
     */
	bool updatePostMovement();

	/**
	 * Retrieves a list of the nearby agents and derives information about it
     */
	void updateNearbyAgents();

	/**
	 * Derives and stores information about the nearby driver
	 *
     * @param nearbyAgent the pointer to the agent which owns the driver role
     * @param nearbyDriver the pointer to the driver role object
     *
	 * @return
     */
	bool updateNearbyAgent(const Agent *nearbyAgent, const Driver *nearbyDriver);

	/**
	 * Sets the current traffic signal based on the end node of the current link.
     */
	void setTrafficSignal();

	/**
	 * Sets the parameters related to the traffic signal. (Colour, distance to traffic signal)
	 * 
     * @param params the update parameters
     */
	void setTrafficSignalParams(DriverUpdateParams &params);

	/**
	 * This method is used to check if there is enough space on the lane where a vehicle from the
	 * loading queue wants to start its journey.
	 * 
     * @return true if empty space is found, else false
     */
	bool findEmptySpaceAhead();

	/**
	 * This method simply increments the vehicle count for the vehicle's current road segment in the RdSegDensityMap  
     */
	void updateDensityMap();

	void startRdSegStat(const RoadSegment* roadSegment, double startTime);

	void finalizeRdSegStat(const RoadSegment* roadSegment,double endTime, const std::string travelMode);

	/**
	 * This method is used to update the travel times of segments passed by the driver during the current frame tick
	 *
	 * @param segmentsPassed Segments passed by the driver during the current frame tick
	 */
	void updateRoadSegmentTravelTime(const std::vector<const RoadSegment*>& segmentsPassed);

protected:
	/**Pointer to the lane changing model being used*/
	LaneChangingModel *lcModel;

	/**Pointer to the car following model being used*/
	CarFollowingModel *cfModel;

	/**Pointer to the intersection driving model being used*/
	IntersectionDrivingModel* intModel;

	/**Pointer to the intersection driving model previously used (and which may be needed again)*/
	IntersectionDrivingModel* intModelBkUp;

	/**The speed which the vehicle will try to achieve.*/
	double targetSpeed;

	/**
	 * Updates the position of the driver
	 *
     * @param params the update parameters
     *
	 * @return the distance by which we have overflowed into the intersection, 0 otherwise
     */
	virtual double updatePosition(DriverUpdateParams &params);

	/**
	 * Applies the driving behaviour models and gets the acceleration
     * 
	 * @param params the update parameters
     */
	void applyDrivingModels(DriverUpdateParams &params);

	/**
	 * Checks if we need to stop ahead (Bus stop/AMOD pick-up/AMOD Drop-off)
	 * 
     * @param params
     */
	void checkForStoppingPoints(DriverUpdateParams &params);

	/**
	 * Finds the nearest stopping point with in the perception distance and returns the distance to it
	 *
     * @param perceptionDistance the look ahead distance
     * 
	 * @return the distance to the nearest stopping point
     */
	double getDistanceToStopPoint(double perceptionDistance);

	/**
	 * Drives to the new position based on its current location, speed and acceleration. Also updates the speed and acceleration
	 *
     * @param params
     *
	 * @return overflow distance into the intersection if any, 0 otherwise
     */
	double drive(DriverUpdateParams &params);

	/**
	 * Calculates the dwell time of a vehicle at a location (eg. bus at a bus stop)
	 * 
     * @param A
     * @param B
     * @param delta_bay
     * @param delta_full
     * @param Pfront
     * @param no_of_passengers
     *
	 * @return
     */
	virtual double dwellTimeCalculation(int A, int B, int delta_bay, int delta_full, int Pfront, int no_of_passengers);

	/**
	 * Initialises the path of the vehicle from the current location to the next activity location.
	 * Also, Creates and allocates a new vehicle if requested
	 *
     * @param createVehicle indicates whether a new vehicle is to be created
     *
	 * @return Returns the new vehicle, if requested to build one
     */
	Vehicle* initializePath(bool createVehicle);

	/**
	 * Sets the initial values of the parameters at the origin of the trip.
	 * 
     * @param p
     */
	void setOrigin(DriverUpdateParams &p);

	/**
	 * Updates the perceptions of the given nearest vehicle
	 * 
     * @param nearestVehicle the nearest vehicle
     * @param params the driver parameters
     */
	void perceivedDataProcess(NearestVehicle &nearestVehicle, DriverUpdateParams &params);

	/**
	 * Returns the angle (orientation) of the vehicle. Used for displaying on the visualiser only
     * @return
     */
	double getAngle() const;

public:
	/**The DriverPathMover object*/
	DriverPathMover fwdDriverMovement;

	/**perform incident response*/
    IncidentPerformer incidentPerformer;

	explicit DriverMovement();
	virtual ~DriverMovement();

	/**
	 * Initialises the driver movement
     */
	virtual void init();

	/**
	 * This method is called for the first tick of the agent's role as a driver. The method initialises the path,
	 * allocates a vehicle to the driver and does other initialisation tasks
     */
	virtual void frame_init();

	/**
	 * This method is called every frame tick and is responsible for the driver's movement and all decisions leading to
	 * the movement during the tick
     */
	virtual void frame_tick();

	/**
	 * This method outputs the parameters that changed at the end of the tick
     */
	virtual std::string frame_tick_output();

	/**
	 * Marks the start time and origin
     * 
	 * @return the updated travel metric structure
     */
	virtual TravelMetric & startTravelTimeMetric();

	/**
	 * Marks the destination and end time and travel time
     *
	 * @return the updated travel metric structure
     */
	virtual TravelMetric & finalizeTravelTimeMetric();

	/**
	 * This method computes the density at every road segment and outputs it to file
	 * 
     * @param tick the time for which the densities are being output
     */
	static void outputDensityMap(unsigned int tick);

	/**
	 * Sets the position of the parent object. This position is used by the AuraManager to build and update the spatial tree
	 * required for spatial searching
	 */
	void setParentBufferedData();

	/**
	 * Builds a path of way-points consisting of road segments and turning groups using the given way-points, 
	 * starting road segment and starting lane
	 *
     * @param wayPoints The list of way points along the path
	 * @return the path consisting of segments and turning groups (as way points)
     */
	std::vector<WayPoint> buildPath(std::vector<WayPoint> &wayPoints);

	/**
	 * Replaces the path with the given path
	 * 
     * @param path the new path
     */
	void resetPath(std::vector<WayPoint> path);	

	/**
	 * Checks if the current segment is the last segment in the link
     *
	 * @return true if the segment is the last in the link, else false
     */
	bool isLastSegmentInLink() const;

	/**Returns the current position of the driver*/
	Point getPosition();	

	/**
	 * Checks if a connection exists between the given lane and segment, either in the form of a lane connector or a turning path.
	 *
     * @param fromLane
     * @param toSegment
	 * 
     * @return true, if the lane is connected to the segment
     */
	bool isLaneConnectedToSegment(const Lane *fromLane, const RoadSegment *toSegment);

	/**
	 * Identifies the lanes adjacent to the current lane
	 * 
     * @param params the driver parameters to be updated with the new information
     */
	void identifyAdjacentLanes(DriverUpdateParams &params);

	/**
	 * Performs the lateral movement done while changing lanes
	 * 
     * @param params the update parameters
     */
	void updateLateralMovement(DriverUpdateParams &params);

	/**
	 * Synchronises the lane information after completion of lane changing movement
	 * 
     * @param params the update parameters
     */
	void syncLaneInfoPostLateralMove(DriverUpdateParams &params);

	/**
	 * Reroutes around a given blacklisted set of links
	 * 
     * @param blacklisted the blacklisted links
     */
	void rerouteWithBlacklist(const std::vector<const Link *> &blacklisted);

	/**
	 * Reroutes the vehicle from its current position with the given path.
	 *
     * @param path the new path
     */
	void rerouteWithPath(const std::vector<WayPoint>& path);

	Driver* getParentDriver() const
	{
		return parentDriver;
	}

	void setParentDriver(Driver* parentDriver)
	{
		if (!parentDriver)
		{
			throw std::runtime_error("parentDriver cannot be NULL");
		}

		safe_delete_item(this->parentDriver);
		this->parentDriver = parentDriver;
	}

	CarFollowingModel* getCarFollowModel()
	{
		return cfModel;
	}
};
}
