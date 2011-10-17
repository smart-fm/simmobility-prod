#pragma once

#include <cmath>

namespace sim_mob
{

/**
 * Lightwight double-point struct
 */
struct DPoint {
	double x;
	double y;
	DPoint(double x, double y) : x(x), y(y) {}
};


/**
 * Simple, lightweight class to represent vector operations
 */
class DynamicVector {
private:
	DPoint pos;
	DPoint mag;

public:
	DynamicVector(double fromX=0.0, double fromY=0.0, double toX=0.0, double toY=0.0)
		: pos(fromX, fromY), mag(toX-fromX, toY-fromY) {}
	DynamicVector(const DynamicVector& copyFrom)
		: pos(copyFrom.pos.x, copyFrom.pos.y), mag(copyFrom.mag.x, copyFrom.mag.y) {}

	//Basic accessors
	double getX() const { return pos.x; }   ///<Retrieve the x coordinate of the origin.
	double getY() const { return pos.y; }   ///<Retrieve the y coordinate of the origin.
	double getEndX() const { return pos.x + mag.x; }  ///<Retrieve the x coordinate of the origin+magnitude.
	double getEndY() const { return pos.y + mag.y; }  ///<Retrieve the y coordinate of the origin+magnitude.
	double getMagnitude() const { return sqrt(mag.x*mag.x + mag.y*mag.y); }  ///<Retrieve the magnitude.

	//Basic utility functions
	DynamicVector& translateVect(double dX, double dY) { pos.x += dX; pos.y += dY; return *this; }  ///<Shift this vector by dX,dY. (Moves the origin)
	DynamicVector& translateVect() { return translateVect(mag.x, mag.y); }  ///<Shift this vector by its own magnitude. Effectively moves the vector's origin to its "end" point.
	DynamicVector& scaleVectTo(double val) { ///<Scale this vector's magnitude TO a given value. (Note that this vector need not be a unit vector.)
		if (mag.x==0 && mag.y==0) {
			//Nothing to do; avoid dividing by NaN
			return *this;
		}

		//Note: The old way (converting to a unit vector then scaling) is very likely
		//      to introduce accuracy errors, since 1 "unit" is a very small number of centimeters.
		//      That is why this function factors in the unit vector in the same step.
		double factor = val/getMagnitude(); //Dividing first is usually slightly more accurate
		mag.x = factor*mag.x;
		mag.y = factor*mag.y;
		return *this;
	}

	//Slightly more complex
	DynamicVector& flipMirror(){ mag.x=-mag.x; mag.y=-mag.y; return *this;}  ///<Flip this vector 180 degrees around the origin.
	DynamicVector& flipNormal(bool clockwise) {  ///<Flip this vector 90 degrees around the origin, either clockwise or counter-clockwise.
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


