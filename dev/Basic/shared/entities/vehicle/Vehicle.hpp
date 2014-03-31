//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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
#include <vector>

#include "conf/settings/DisableMPI.h"

#include "util/MovementVector.hpp"
#include "util/DynamicVector.hpp"
#include "geospatial/streetdir/WayPoint.hpp"
#include "geospatial/GeneralPathMover.hpp"
#include "geospatial/Lane.hpp"

namespace sim_mob {

class PackageUtils;
class UnPackageUtils;

/*// Commented out by Vahid
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
};*/

class FMODSchedule;
class Vehicle {
public:
//	Vehicle(int startLaneID);
	Vehicle(double length, double width); //TODO: now that the constructor is non-default, we might be able to remove throw_if_error()
	Vehicle(int vehicle_id, double length, double width); //Test
//	Vehicle();  //There is no wpPoint to initialize one Vehicle when crossing
	Vehicle(const Vehicle& copy); ///<Copy constructor

	//Enable polymorphism
	virtual ~Vehicle(){}

public:
	const double length;  ///<length of the vehicle
	const double width;   ///<width of the vehicle
	bool isQueuing; 	 ///<for mid-term use
	FMODSchedule* schedule;
	DPoint currPos;

	double getLateralMovement() const;         ///<Retrieve a value representing how far to the LEFT of the current lane the vehicle has moved.
	double getVelocity() const;      ///<Retrieve forward velocity.
	double getLatVelocity() const;   ///<Retrieve lateral velocity.
	double getAcceleration() const;  ///<Retrieve forward acceleration.
	bool isInIntersection() const;   ///<Are we now in an intersection?
	bool isDone() const; ///<Are we fully done with our path?
	bool hasPath() const; ///<Do we have a path to move on?

	//Special
	LANE_CHANGE_SIDE getTurningDirection() const;

	//More stuff; some might be optional.
	const sim_mob::RoadSegment* getCurrSegment() const;
	const sim_mob::RoadSegment* getNextSegment(bool inSameLink=true) const;
	const sim_mob::RoadSegment* getSecondSegmentAhead();
	const sim_mob::RoadSegment* getPrevSegment(bool inSameLink=true) const;
	const sim_mob::RoadSegment* hasNextSegment(bool inSameLink) const;
	const sim_mob::Lane* getCurrLane() const;
	void setPositionInIntersection(double x, double y);
	DPoint& getPositionInIntersection();
	void setTurningDirection(LANE_CHANGE_SIDE direction);
	//Modifiers
	void setVelocity(double value);      ///<Set the forward velocity.
	void setLatVelocity(double value);   ///<Set the lateral velocity.
	void setAcceleration(double value);  ///<Set the forward acceleration.
//	double moveFwd(double amt);            ///<Move this car forward. Automatically moved it to new Segments unless it's in an intersection.
	void moveFwd_med(double amt);
	void actualMoveToNextSegmentAndUpdateDir_med();		//~melani for mid-term
	void moveLat(double amt);            ///<Move this car laterally. NOTE: This will _add_ the amt to the current value.
	void resetLateralMovement();         ///<Put this car back in the center of the current lane.

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
//	DPoint getPosition() const;
	/*needed by mid-term*/
	double getPositionInSegment();
	void setPositionInSegment(double newDist2end);
	//unit cm, this is based on lane zero's polypoints

private:


	//NOTE: The error state is a temporary sanity check to help me debug this class. There are certainly
	//      better ways to handle this (e.g., non-default constructor).
	bool error_state;
	void throw_if_error() const {
		if (error_state) {
			throw std::runtime_error("Error: can't perform certain actions on an uninitialized vehicle.");
		}
	}

	//Serialization-related friends
	friend class PackageUtils;
	friend class UnPackageUtils;
};

} // namespace sim_mob
