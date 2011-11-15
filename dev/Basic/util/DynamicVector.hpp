/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <cmath>
#include <stdexcept>

namespace sim_mob
{

/**
 * Lightwight double-point struct
 */
struct DPoint {
	double x;
	double y;
	DPoint(double x=0.0, double y=0.0) : x(x), y(y) {}
};


/**
 * Simple, lightweight class to represent vector operations
 */
class DynamicVector {
private:
	DPoint pos;
	DPoint mag;

	//Helper variable; if set, it is assumed that mag==(0,0). In fact, mag remains at its previous
	// value so that scaling to zero and back produces a valid vector. The alternative is to maintain
	// the vector's angle, but this would be more costly.
	bool isZero;

public:
	DynamicVector(double fromX=0.0, double fromY=0.0, double toX=0.0, double toY=0.0)
		: pos(fromX, fromY), mag(toX-fromX, toY-fromY) {
		isZero = toX==fromX && toY==fromY; //In this case, there's not much we can do.
	}
	DynamicVector(const DynamicVector& copyFrom)
		: pos(copyFrom.pos.x, copyFrom.pos.y), mag(copyFrom.mag.x, copyFrom.mag.y), isZero(copyFrom.isZero) {}

	//Basic accessors
	double getX() const { return pos.x; }   ///<Retrieve the x coordinate of the origin.
	double getY() const { return pos.y; }   ///<Retrieve the y coordinate of the origin.

	///Retrieve the x coordinate of the origin+magnitude.
	double getEndX() const { return pos.x + (isZero?0:mag.x); }

	///Retrieve the y coordinate of the origin+magnitude.
	double getEndY() const { return pos.y + (isZero?0:mag.y); }

	///Retrieve the magnitude.
	double getMagnitude() const { return isZero?0:sqrt(mag.x*mag.x + mag.y*mag.y); }

	//Retrieve the angle. Should still work even if the magnitude is zero
	//Returns an angle in the range 0..2*PI
	double getAngle() const {
		if(mag.x==0 && mag.y==0) {
			//This only happens if the vector is specifically initialize with zero size.
			throw std::runtime_error("Impossible to retrieve a vector's angle if it has never had a size.");
		}

		return atan2(mag.y, mag.x) + M_PI;
	}

	//Basic utility functions
	DynamicVector& translateVect(double dX, double dY) { pos.x += dX; pos.y += dY; return *this; }  ///<Shift this vector by dX,dY. (Moves the origin)
	DynamicVector& translateVect() { return translateVect((isZero?0:mag.x), (isZero?0:mag.y)); }  ///<Shift this vector by its own magnitude. Effectively moves the vector's origin to its "end" point.

	///Scale this vector's magnitude TO a given value. (Note that this vector need not be a unit vector.)
	DynamicVector& scaleVectTo(double val) {
		//Will this vector have no size after this operation?
		isZero = (val==0);

		//If non-zero, we can scale it.
		if (!isZero) {
			if(mag.x==0 && mag.y==0) {
				//This only happens if the vector is specifically initialize with zero size.
				throw std::runtime_error("Impossible to scale a vector that has never had a size.");
			}

			//Note: The old way (converting to a unit vector then scaling) is very likely
			//      to introduce accuracy errors, since 1 "unit" is a very small number of centimeters.
			//      That is why this function factors in the unit vector in the same step.
			double factor = val/getMagnitude(); //Dividing first is usually slightly more accurate
			mag.x = factor*mag.x;
			mag.y = factor*mag.y;
		}
		return *this;
	}

	//Slightly more complex
	///Flip this vector 180 degrees around the origin.
	DynamicVector& flipMirror(){
		//Note: Mirroring a vector while isZero is true is fine; the change will be visible later once the vector scales to a non-zero size.
		mag.x=-mag.x;
		mag.y=-mag.y;
		return *this;
	}

	///Flip this vector 90 degrees around the origin, either clockwise or counter-clockwise.
	DynamicVector& flipNormal(bool clockwise) {
		//Note: Flipping a vector while isZero is true is fine; the change will be visible later once the vector scales to a non-zero size.
		int sign = clockwise ? 1 : -1;
		double newX = mag.y*sign;
		double newY = -mag.x*sign;
		mag.x = newX;
		mag.y = newY;
		return *this;
	}
	DynamicVector& flipRight() { return flipNormal(true); }  ///<Flip this vector 90 degrees clockwise around the origin.
	DynamicVector& flipLeft() { return flipNormal(false); }  ///<Flip this vector 90 degrees counter-clockwise around the origin.

};


}


