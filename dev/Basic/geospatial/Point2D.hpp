/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once


namespace sim_mob
{

/**
 * Simple storage class for geospatial items in the road network. In general, everything within a
 * road network is defined in reference to its Point2D location. Road segments are defined by 2
 * nodes (each node containing a point):  one at the beginning of the segment and another at the end.
 * Intersections are defined by a point at the intersection of all roads that join at that point.
 *
 * The Point2D location of any item should be accurate enough to generate a reasonable visualization
 * of that item using its Point(s) alone.
 */
class Point2D {
public:
	Point2D(double xPos=0.0, double yPos=0.0) : xPos(xPos), yPos(yPos) {}

	double getX() const { return xPos; }
	double getY() const { return yPos; }


private:
	double xPos;
	double yPos;

};





}
