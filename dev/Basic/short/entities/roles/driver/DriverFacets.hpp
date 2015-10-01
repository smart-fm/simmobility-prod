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

#include "conf/settings/DisableMPI.h"
#include "Driver.hpp"
#include "DriverPathMover.hpp"
#include "DriverUpdateParams.hpp"
#include "entities/amodController/AMODController.hpp"
#include "entities/IncidentStatus.hpp"
#include "entities/roles/driver/models/CarFollowModel.hpp"
#include "entities/roles/Role.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "geospatial/Incident.hpp"
#include "geospatial/RoadItem.hpp"
#include "IncidentPerformer.hpp"
#include "util/OneTimeFlag.hpp"


namespace sim_mob
{

class CarFollowModel;

class DriverBehavior : public sim_mob::BehaviorFacet
{
protected:
	Driver* parentDriver;

public:
	explicit DriverBehavior();
	virtual ~DriverBehavior();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

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
};

class DriverMovement : public sim_mob::MovementFacet
{
private:
	//The driver whose movement is being simulated by the DriverMovement object
	Driver* parentDriver;

	//The traffic signal at the approaching intersection. If the intersection is
	//un-signalised, this will be null
	const Signal* trafficSignal;

	//The index of the target lane. The target lane is the lane we want to be in after crossing the
	//intersection (In short, this is the index of the lane pointed to by nextLaneInNextLink)
	size_t targetLaneIndex;

	//The pointer to the lane in the next link which is connected to our current lane
	const Lane* nextLaneInNextLink;

	//Map of road segment vs the aggregate vehicle count over the collection interval
	static map<const RoadSegment *, unsigned long> rdSegDensityMap;

	//Mutex to lock the density map
	static boost::mutex densityUpdateMutex;

	//For generating a debugging trace
	mutable std::stringstream DebugStream;

	//Have we sent the list of all regions at least once?
	OneTimeFlag sentAllRegions;

	//The most recently-set path, which will be sent to RoadRunner.
	std::vector<const sim_mob::RoadSegment*> rrPathToSend;

	//Sets the distance and the driver of the NearestVehicle object given. The distance is the distance between
	//the current driver and the other driver
	void setNearestVehicle(NearestVehicle& res, double distance, const Vehicle* veh, const Driver* other);

	//Updates the perceived values
	void perceiveParameters(DriverUpdateParams& p);

	//Updates the information that is sensed. Such as, the positions of nearby vehicles, the traffic signal
	bool updateSensors(timeslice now);

	//Moves the vehicle forward according to the accelerations and velocities calculated by the various
	//driver models
	bool updateMovement(timeslice now);

	//Deals with the effect of moving forward - Chooses next lane in next link, checks if we're approaching an
	//intersection, calculates the intersection trajectory if we've moved into an intersection
	bool updatePostMovement(timeslice now);

	//Returns true if there is a pedestrian on the crossing when the light has turned green, else
	//returns false
	bool isPedestrianOnTargetCrossing() const;

	//Sets the nextLaneInNextLink based on the current lane and the connections to the next road segment
	void chooseNextLaneForNextLink(DriverUpdateParams& p);

	//Calculates the trajectory that the vehicle needs to follow within the intersection
	void calculateIntersectionTrajectory(DPoint movingFrom, double overflow);

	//Updates the information about the current lane and the neighbouring lanes
	void syncCurrLaneCachedInfo(DriverUpdateParams& p);

	//Updates the position of the driver on the new lane (which we end up on after exiting the intersection)
	void postIntersectionDriving(DriverUpdateParams& p);

	//Retrieves a list of the nearby agents and derives information about it
	void updateNearbyAgents();

	//Derives information about the nearby driver
	bool updateNearbyAgent(const sim_mob::Agent* other, const sim_mob::Driver* other_driver);

	//Derives information about the nearby pedestrian
	void updateNearbyAgent(const sim_mob::Agent* other, const sim_mob::Pedestrian* pedestrian);

	//Sets the current traffic signal based on the end node of the current Road Segment.
	void setTrafficSignal();

	//Sets the parameters related to the traffic signal. (Colour, distance to traffic signal)
	void setTrafficSignalParams(DriverUpdateParams& p);

	//Performs driving within the intersection.
	void performIntersectionDriving(DriverUpdateParams& p);

	//Returns the distance to the end of the segment
	double getDistanceToSegmentEnd() const;

	//Returns the current poly-line vector
	sim_mob::DynamicVector getCurrPolylineVector() const;

	//Check if there is enough space on the lane where a vehicle from the loading queue wants to start its journey
	bool findEmptySpaceAhead();

	//This method updates the segment density map
	void updateDensityMap();

	//This method helps defines the driver behaviour when approaching an unsignalised intersection
	double performIntersectionApproach();

protected:
	//Pointer to the lane changing model being used
	LaneChangeModel* lcModel;

	//Pointer to the car following model being used
	CarFollowModel* cfModel;

	//Pointer to the intersection driving model being used
	IntersectionDrivingModel* intModel;

	//Pointer to the intersection driving model previously used (and which may be needed again)
	IntersectionDrivingModel* intModelBkUp;

	//The speed which the vehicle will try to achieve.
	double targetSpeed;

	//Updates the position of the driver on the link
	virtual double updatePositionOnLink(DriverUpdateParams& p);

	//Applies the lane changing and car following models and determines the current state of the
	//vehicle i.e. accelerating/decelerating/changing lane/etc
	void calcVehicleStates(DriverUpdateParams& p);

	//Calculates the distance to the stopping point
	void calcDistanceToStoppingPoint(DriverUpdateParams& p);

	//Calculates the new position and speed after based on its current location, speed and acceleration.
	double move(DriverUpdateParams& p);

	//Calculates the dwell time of a vehicle at a location (eg. bus at a bus stop)
	virtual double dwellTimeCalculation(int A, int B, int delta_bay, int delta_full, int Pfront,
										int no_of_passengers);

	//Initialises the path of the vehicle. Creates and allocates a new vehicle if requested
	sim_mob::Vehicle* initializePath(bool allocateVehicle);

	//Sets the initial values of the parameters at the origin of the trip.
	void setOrigin(DriverUpdateParams& p);

	///Set the internal rrRegions array from the current path.
	///This effectively converts a list of RoadSegments into a (much smaller) list of Regions.
	///This will trigger communication with the client.
	void setRR_RegionsFromCurrentPath();

	//Returns the nearest vehicle
	//NearestVehicle& nearestVehicle(DriverUpdateParams& p);

	//Updates the perceptions of the given nearest vehicle
	void perceivedDataProcess(NearestVehicle & nv, DriverUpdateParams& params);

	//Returns the angle (orientation) of the vehicle.
	//Used for displaying on the visualiser only
	double getAngle() const;

public:

	//The DriverPathMover object
	DriverPathMover fwdDriverMovement;

	//perform incident response
	IncidentPerformer incidentPerformer;

	//Constructor
	explicit DriverMovement(sim_mob::Person* parentAgent = nullptr, Driver* parentDriver = nullptr);

	//Outputs the road segment densities
	static void outputDensityMap(unsigned int tick);

	//Returns the pointer to the driver object associated with this movement
	Driver* getParentDriver() const
	{
		return parentDriver;
	}

	//Sets the driver object associated with this movement
	void setParentDriver(Driver* parentDriver)
	{
		if (!parentDriver)
		{
			throw std::runtime_error("parentDriver cannot be NULL");
		}

		safe_delete_item(this->parentDriver);
		this->parentDriver = parentDriver;
	}

	//Returns the pointer to the car following model associated with this movement
	CarFollowModel* getCarFollowModel()
	{
		return cfModel;
	}

	//Sets the position and velocity values owned by the Agent object
	void setParentBufferedData();

	//Builds a path consisting of road segments using the given way-points and starting lane
	void buildAndSetPath(std::vector<sim_mob::WayPoint> wp_path, int startLaneID);

	//Builds a path consisting of road segments using the given way-points, starting segment and starting lane
	void buildAndSetPathWithInitSeg(std::vector<sim_mob::WayPoint> wp_path, int startLaneID, int segId, int initPer,
									int initSpeed);

	//Reset the path
	void resetPath(std::vector<sim_mob::WayPoint> wp_path);

	//Returns true if the path contains the next segment, else returns false. The boolean parameter
	//allows us to check within the link or in the next link
	const sim_mob::RoadSegment* hasNextSegment(bool inSameLink) const;

	//Returns the current position of the driver
	DPoint getPosition();

	//Gets the nearest obstacle within the given perceptionDis
	const sim_mob::RoadItem* getRoadItemByDistance(sim_mob::RoadItemType type, double &dis, double perceptionDis = 20000,
												   bool isInSameLink = true);

	//Gets the distance to nearest forward stop point in the link
	//Returns -1 if stopping point was not found, returns value > 0 when the stopping point
	//was found. This value is the distance to the stop point
	double getDisToStopPoint(double perceptionDis = 20000);

	//Gets the lanes connected to the segment within the look ahead distance
	//The parameter lanePool stores the result
	void getLanesConnectToLookAheadDis(double distance, std::vector<sim_mob::Lane*>& lanePool);

	//Returns true if the lane is connected to the segment
	bool isLaneConnectedToSegment(sim_mob::Lane* lane, const sim_mob::RoadSegment* rs);

	//Updates the information known about the adjacent lanes
	void updateAdjacentLanes(DriverUpdateParams& p);

	//Performs the lateral movement done while changing lanes
	void updateLateralMovement(DriverUpdateParams& p);

	//Synchronises the lane information after completion of lane changing movement
	void syncInfoLateralMove(DriverUpdateParams& p);

	///Reroutes around a given blacklisted set of RoadSegments. See Role for documentation.
	void rerouteWithBlacklist(const std::vector<const sim_mob::RoadSegment*>& blacklisted);

	//Reroutes the vehicle from its current position with the given path.
	void rerouteWithPath(const std::vector<sim_mob::WayPoint>& path);

	//Updates the intersection velocity
	void updateIntersectionVelocity();

	/*Overridden functions*/

	virtual ~DriverMovement();

	//Initialises the driver movement
	virtual void init();

	virtual void frame_init();

	virtual void frame_tick();

	virtual void frame_tick_output();

	// mark startTimeand origin
	virtual TravelMetric & startTravelTimeMetric();

	//	mark the destination and end time and travel time
	virtual TravelMetric & finalizeTravelTimeMetric();
};
}
