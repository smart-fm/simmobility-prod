/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "geospatial/Point2D.hpp"
#include "DynamicVector.hpp"

namespace sim_mob
{

/**
 * Simple class to handle scaling along a vector.
 *
 * \note
 * I'm making this header-only for now to make it simpler to develop. Will need to migrate to its own
 * separate *.cpp file if nothing breaks.
 */
class MovementVector {
public:
	MovementVector(const DynamicVector& vect=DynamicVector(), double amt=0.0) : vect(vect), amount(amt) {
		lat = DynamicVector(0, 0, vect.getEndX()-vect.getX(), vect.getEndY()-vect.getY());
		lat.flipLeft();
		resetLateral();
	}

	//Retrieval functions
	double getX() const {
		return getPos().x;
	}
	double getY() const {
		return getPos().y;
	}
	double getAmountMoved() const {
		return std::min(std::max(0.0,amount), vect.getMagnitude());
	}
	double getOverflow() const {
		return std::max(0.0, amount-vect.getMagnitude());
	}
	double getLateralMovement() const {
		return lat.getMagnitude();
	}
	bool reachedEnd() const {
		return getOverflow() > 0.0;
	}

	//Moving along this vector.
	void moveFwd(double amt) {
		amount += amt;
	}
	void moveLat(double amt) {
		lat.scaleVectTo(lat.getMagnitude()+amt);
	}
	void resetLateral() {
		lat.scaleVectTo(0.0);
	}

	//Start moving on another vector. Carry over any overflow when you switch.
	void moveToNewVect(const DynamicVector& newVect) {
		amount = getOverflow();
		vect = newVect;
	}

private:
	DynamicVector vect;  //The vector we are moving along.
	DynamicVector lat;   //Lateral movement is generally more free.

	//How far along this vector we are. Overflow is handled later.
	double amount;

	//Helper function
	DPoint getPos() const {
		DynamicVector temp(vect);
		double adjustedMag = std::min(std::max(0.0,amount), temp.getMagnitude());
		temp.scaleVectTo(adjustedMag);
		return DPoint(temp.getEndX()+lat.getEndX(), temp.getEndY()+lat.getEndY());
	}
};


}


