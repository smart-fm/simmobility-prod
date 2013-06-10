//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <iostream>
//add by xuyan
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

namespace sim_mob
{

/**
 * Simple storage class for geospatial items in the road network.
 *
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 * \author Xu Yan
 *
 * In general, everything within a
 * road network is defined in reference to its Point2D location. Road segments are defined by 2
 * nodes (each node containing a point):  one at the beginning of the segment and another at the end.
 * Intersections are defined by a point at the intersection of all roads that join at that point.
 *
 * The Point2D location of any item should be accurate enough to generate a reasonable visualization
 * of that item using its Point(s) alone.
 *
 * X and Y position are defined in centimeters. Due to the current projection of any given zone, it
 * is perfectly normal for x/y to be negative.
 */
class Point2D {
public:
	explicit Point2D(int xPos=0, int yPos=0) : xPos(xPos), yPos(yPos) {}

	int getX() const { return xPos; }
	int getY() const { return yPos; }

	void setX(int xPos_)  { xPos = xPos_; }
	void setY(int yPos_)  { yPos = yPos_; }


private:
	int xPos;
	int yPos;

public:
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version);

	friend class boost::serialization::access;

	bool nearToPoint(Point2D another, double distance) const;
};


//Non-member output and comparison functions
//NOTE: These used to be inline, but << doesn't need it (console output is slow anyway)
//      and == and != can be made into inline member functions if this is actually required.
std::ostream& operator<<(std::ostream& stream, Point2D const & point);
bool operator==(Point2D const & p1, Point2D const & p2);
bool operator!=(Point2D const & p1, Point2D const & p2);


// Template implementation.
template<class Archive>
void Point2D::serialize(Archive & ar, const unsigned int version)
{
	ar & xPos;
	ar & yPos;
}




}
