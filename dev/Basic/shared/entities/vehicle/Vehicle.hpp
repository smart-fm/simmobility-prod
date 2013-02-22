/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file Vehicle.hpp
 *
 * \author Li Zhemin
 * \author Seth N. Hetu
 * \author Zhang Shuai
 * \author Xu Yan
 */

#pragma once

#include <stdexcept>
#include <sstream>

#include "util/MovementVector.hpp"
#include "util/DynamicVector.hpp"
#include "geospatial/GeneralPathMover.hpp"
#include "geospatial/Lane.hpp"

namespace sim_mob {

class PackageUtils;
class UnPackageUtils;

class Park
{
	bool parkingEnabled;
	double parkingTime;
	double elapsedParkingTime;
public:
	Park(double parkingTime_,bool parkingEnabled_ = true) : parkingTime(parkingTime_), parkingEnabled(parkingEnabled_), elapsedParkingTime(0){}
	void enableParking() { parkingEnabled = true; }
	void disableParking() { parkingEnabled = false; }
	bool isParkingEnabled(){ return parkingEnabled;}
	void setParkingTime(double time) { parkingTime = time; }
	double getParkingTime() { return parkingTime; }
	void incrementElapsedParkingTime(double time) { elapsedParkingTime += time;}
	void setElapsedParkingTime(double time) { elapsedParkingTime = time;}
	double getElapsedParkingTime() { return elapsedParkingTime;}
	bool isparkingTimeOver() const {
//		std::cout << "isparkingTimeOver()::elapsedParkingTime =" << elapsedParkingTime << "   parkingTime = " << parkingTime << std::endl;
		return elapsedParkingTime >= parkingTime; }
};

class Vehicle {
public:
	Vehicle(std::vector<sim_mob::WayPoint> wp_path, int startLaneID);
	Vehicle(std::vector<sim_mob::WayPoint> wp_path, int startLaneID, double length, double width); //TODO: now that the constructor is non-default, we might be able to remove throw_if_error()
	Vehicle(std::vector<const RoadSegment*> path, int startLaneID, int vehicle_id, double length, double width); //Test
	Vehicle();  //There is no wpPoint to initialize one Vehicle when crossing
	Vehicle(const Vehicle& copy); ///<Copy constructor

	//Enable polymorphism
	virtual ~Vehicle(){}

public:
	const double length;  ///<length of the vehicle
	const double width;   ///<width of the vehicle
	bool isQueuing; 	 ///<for mid-term use
	//Call once
	void initPath(std::vector<sim_mob::WayPoint> wp_path, int startLaneID);

	void resetPath(std::vector<sim_mob::WayPoint> wp_path);

	//Accessors
	double getX() const;   ///<Retrieve the vehicle's absolute position, x
	double getY() const;   ///<Retrieve the vehicle's absolute position, y
	double getDistanceMovedInSegment() const;   ///<Retrieve the total distance moved in this segment so far.
	double getDistanceToSegmentStart() const;

	// segment length is based on lane's polypoints , which lane? it is a problem...
	// be careful, it is not relate to segment's start ,end nodes
	// unit cm
 	double getCurrentSegmentLength();

	//double getCurrLaneLength() const; ///<Get the total length of this RoadSegment.

	double getLateralMovement() const;         ///<Retrieve a value representing how far to the LEFT of the current lane the vehicle has moved.
	double getVelocity() const;      ///<Retrieve forward velocity.
	double getLatVelocity() const;   ///<Retrieve lateral velocity.
	double getAcceleration() const;  ///<Retrieve forward acceleration.
	//bool reachedSegmentEnd() const;  ///<Return true if we've reached the end of the current segment.
	bool isInIntersection() const;   ///<Are we now in an intersection?
	bool isDone() const; ///<Are we fully done with our path?
	bool hasPath() const; ///<Do we have a path to move on?
	bool isMovingForwardsInLink() const;

	//Special
	int getVehicleID() const;
	double getAngle() const;  ///<For display purposes only.
	LANE_CHANGE_SIDE getTurningDirection() const;

	//Helper method; used in BusDriver
	const std::vector<const sim_mob::RoadSegment*>& getCompletePath() const;

	//More stuff; some might be optional.
	const sim_mob::RoadSegment* getCurrSegment() const;
	const sim_mob::RoadSegment* getNextSegment(bool inSameLink=true) const;
	const sim_mob::RoadSegment* getSecondSegmentAhead();
	const sim_mob::RoadSegment* getPrevSegment(bool inSameLink=true) const;
	const sim_mob::RoadSegment* hasNextSegment(bool inSameLink) const;
	sim_mob::DynamicVector getCurrPolylineVector() const;
	const sim_mob::Link* getCurrLink() const;
	const sim_mob::Lane* getCurrLane() const;
	const sim_mob::Node* getNodeMovingTowards() const;
	const sim_mob::Node* getNodeMovingFrom() const;
	double getCurrLinkLaneZeroLength() const;
	double getCurrPolylineLength() const;
	double getAllRestRoadSegmentsLength() const;
	double getCurrLinkReportedLength() const;
	void shiftToNewLanePolyline(bool moveLeft);
	void moveToNewLanePolyline(int laneID);
	void setPositionInIntersection(double x, double y);

	void setTurningDirection(LANE_CHANGE_SIDE direction);
	//Modifiers
	void setVelocity(double value);      ///<Set the forward velocity.
	void setLatVelocity(double value);   ///<Set the lateral velocity.
	void setAcceleration(double value);  ///<Set the forward acceleration.
	double moveFwd(double amt);            ///<Move this car forward. Automatically moved it to new Segments unless it's in an intersection.
	void moveFwd_med(double amt);
	void actualMoveToNextSegmentAndUpdateDir_med();		//~melani for mid-term
	void moveLat(double amt);            ///<Move this car laterally. NOTE: This will _add_ the amt to the current value.
	void resetLateralMovement();         ///<Put this car back in the center of the current lane.
	const Lane* moveToNextSegmentAfterIntersection();   ///<If we're in an intersection, move out of it.
 	//Complex
	//void newPolyline(sim_mob::Point2D firstPoint, sim_mob::Point2D secondPoint)

	//Temporary; workaround
	//const DynamicVector& TEMP_retrieveFwdVelocityVector();

#ifndef SIMMOB_DISABLE_MPI
public:
	///Serialization
	static void pack(PackageUtils& package, Vehicle* one_vehicle);

	static Vehicle* unpack(UnPackageUtils& unpackage);
#endif

//private:
	//temp
private:
	//Trying a slightly more dynamic moving model.
	int vehicle_id;
	GeneralPathMover fwdMovement;
	double latMovement;
	double fwdVelocity;
	double latVelocity;
	double fwdAccel;
	LANE_CHANGE_SIDE turningDirection;

	//Override for when we're in an intersection.
	DPoint posInIntersection;

public:
	DPoint getPosition() const;
	/*needed by mid-term*/
	double getPositionInSegment();
	void setPositionInSegment(double newDist2end);
	//unit cm, this is based on lane zero's polypoints
	 double getNextSegmentLength();
private:


	//NOTE: The error state is a temporary sanity check to help me debug this class. There are certainly
	//      better ways to handle this (e.g., non-default constructor).
	bool error_state;
	void throw_if_error() const {
		if (error_state) {
			throw std::runtime_error("Error: can't perform certain actions on an uninitialized vehicle.");
		}
	}

	//Helper function: Reposition and take care of any remaining velocity.
	/*void repositionAndScale(DynamicVector& item, const DynamicVector& newHeading) {
		double oldMag = item.getMagnitude();
		item = newHeading;
		item.scaleVectTo(oldMag);
	}*/

	//Serialization-related friends
	friend class PackageUtils;
	friend class UnPackageUtils;
};

} // namespace sim_mob
