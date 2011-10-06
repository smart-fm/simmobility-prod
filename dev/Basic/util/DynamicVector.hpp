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
	double getMagnitude() const { return sqrt(mag.x*mag.x + mag.y+mag.y); }

	//Basic utility functions
	DynamicVector& makeUnit()  { return scaleVect(1/getMagnitude()); }
	DynamicVector& scaleVect(double val) { mag.x *= val; mag.y *= val; return *this; }
	DynamicVector& translateVect(double dX, double dY) { pos.x += dX; pos.y += dY; return *this; }
	DynamicVector& translateVect() { return translateVect(mag.x, mag.y); }

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


