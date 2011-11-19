/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Vehicle.h
 *
 *  Created on: Oct 24, 2011
 *      Author: lzm
 */

#pragma once

#include <stdexcept>

#include "util/MovementVector.hpp"
#include "util/DynamicVector.hpp"
#include "entities/roles/driver/GeneralPathMover.hpp"


namespace sim_mob {

class Vehicle {
public:
	Vehicle();

public:
	const double length;  ///<length of the vehicle
	const double width;   ///<width of the vehicle

	//Call once
	void initPath(std::vector<sim_mob::WayPoint> wp_path, int startLaneID);

	//Accessors
	double getX() const;   ///<Retrieve the vehicle's absolute position, x
	double getY() const;   ///<Retrieve the vehicle's absolute position, y
	double getDistanceMovedInSegment() const;   //<Retrieve the total distance moved in this segment so far.
	double getLateralMovement() const;         ///<Retrieve a value representing how far to the LEFT of the current lane the vehicle has moved.
	double getVelocity() const;      ///<Retrieve forward velocity.
	double getLatVelocity() const;   ///<Retrieve lateral velocity.
	double getAcceleration() const;  ///<Retrieve forward acceleration.
	//bool reachedSegmentEnd() const;  ///<Return true if we've reached the end of the current segment.
	bool isInIntersection() const;   ///<Are we now in an intersection?
	bool isDone() const; ///<Are we fully done with our path?
	bool hasPath() const; ///<Do we have a path to move on?

	//Special
	double getAngle() const;  ///<For display purposes only.

	//More stuff; some might be optional.
	const sim_mob::RoadSegment* getCurrSegment() const;
	const sim_mob::RoadSegment* getNextSegment(bool inSameLink=true) const;
	const sim_mob::RoadSegment* getPrevSegment() const;
	const sim_mob::RoadSegment* hasNextSegment(bool inSameLink) const;
	sim_mob::DynamicVector getCurrPolylineVector() const;
	const sim_mob::Link* getCurrLink() const;
	const sim_mob::Node* getNodeMovingTowards() const;
	const sim_mob::Node* getNodeMovingFrom() const;
	double getCurrLinkLength() const;
	void shiftToNewLanePolyline(bool moveLeft);

	//Modifiers
	void setVelocity(double value);      ///<Set the forward velocity.
	void setLatVelocity(double value);   ///<Set the lateral velocity.
	void setAcceleration(double value);  ///<Set the forward acceleration.
	void moveFwd(double amt);            ///<Move this car forward. Automatically moved it to new Segments unless it's in an intersection.
	void moveLat(double amt);            ///<Move this car laterally. NOTE: This will _add_ the amt to the current value.
	void resetLateralMovement();         ///<Put this car back in the center of the current lane.
	const Lane* moveToNextSegmentAfterIntersection();   ///<If we're in an intersection, move out of it.

	//Complex
	//void newPolyline(sim_mob::Point2D firstPoint, sim_mob::Point2D secondPoint)

	//Temporary; workaround
	//const DynamicVector& TEMP_retrieveFwdVelocityVector();


private:
	//Trying a slightly more dynamic moving model.
	//MovementVector position;
	//DynamicVector velocity;
	//DynamicVector velocity_lat; //Lateral velocity. Positive means pointing left.
	//DynamicVector accel;
	GeneralPathMover fwdMovement;
	double latMovement;
	double fwdVelocity;
	double latVelocity;
	double fwdAccel;


	DPoint getPosition() const;


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
};

} // namespace sim_mob

