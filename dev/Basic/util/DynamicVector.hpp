#pragma once


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

	//Basic retrieval
	double getX() { return pos.x; }
	double getY() { return pos.y; }
	double getEndX() { return pos.x + mag.x; }
	double getEndY() { return pos.y + mag.y; }
	double getMagnitude() { return sqrt(mag.x*mag.x + mag.y+mag.y); }

	//Basic utility functions
	void makeUnit()  { scaleVect(1/getMagnitude()); }
	void scaleVect(double val) { mag.x *= val; mag.y *= val; }
	void translateVect(double dX, double dY) { pos.x += dX; pos.y += dY; }
	void translateVect() { translateVect(mag.x, mag.y); }
};


}


