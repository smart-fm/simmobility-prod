#pragma once

#include <cmath>

#include <iostream>

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
 * Very lightweight class to represent vector operations
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

	//Basic retrieval
	double getX() const { return pos.x; }
	double getY() const { return pos.y; }
	double getEndX() const { return pos.x + mag.x; }
	double getEndY() const { return pos.y + mag.y; }
	double getMagnitude() const { return sqrt(mag.x*mag.x + mag.y*mag.y); }

	//Basic utility functions
	//DynamicVector& makeUnit()  { return scaleVect(1/getMagnitude()); }
	DynamicVector& translateVect(double dX, double dY) { pos.x += dX; pos.y += dY; return *this; }
	DynamicVector& translateVect() { return translateVect(mag.x, mag.y); }
	DynamicVector& scaleVectTo(double val) { //Scale any (non-unit) vector
		if (mag.x==0 && mag.y==0) {
			//Nothing to do; avoid dividing by NaN
			return *this;
		}

		//Note: The old way (converting to a unit vector then scaling) is very likely
		//      to introduce accuracy errors, since 1 "unit" is a very small number of centimeters.
		//      That is why this function factors in the unit vector in the same step.
		double factor = val/getMagnitude(); //Dividing first is usually slightly more accurate

		if (fabs(val-11821.5)<1.0) {
			std::cout <<"     mags: " <<mag.x <<"," <<mag.y <<"\n";
			std::cout <<"     factor_comp: " <<val <<"," <<getMagnitude() <<"\n";
			std::cout <<"     factor: " <<factor <<"\n";
		}

		mag.x = factor*mag.x;
		mag.y = factor*mag.y;
		return *this;
	}

	//Slightly more complex
	DynamicVector& flipMirror(){ mag.x=-mag.x; mag.y=-mag.y; return *this;}
	DynamicVector& flipNormal(bool toRight) {
		int sign = toRight ? 1 : -1;
		double newX = mag.y*sign;
		double newY = -mag.x*sign;
		mag.x = newX;
		mag.y = newY;
		return *this;
	}
};


}


