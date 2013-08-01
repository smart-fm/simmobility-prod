/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <cmath>
#include <stdexcept>
#include <iostream>
#include <boost/thread.hpp>

#include "conf/settings/DisableMPI.h"

#include "logging/Log.hpp"

#include "geospatial/Point2D.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#endif


namespace sim_mob
{

/**
 * Lightwight double-point struct
 *
 * \author Seth N. Hetu
 */
struct DPoint {
	double x;
	double y;
	explicit DPoint(double x=0.0, double y=0.0) : x(x), y(y) {}
};

} //namespace sim_mob



////////////////////////////////////////////////
// Serialization for DPoint. This is how one would
//  serialize *outside* the class itself. You can also
//  put the serialize() function inside DPoint; see
//  FixedDelayed<> for an example of this alternate approach.
////////////////////////////////////////////////
#ifndef SIMMOB_DISABLE_MPI
namespace boost {
namespace serialization {
template<class Archive>
void serialize(Archive & ar, sim_mob::DPoint& pt, const unsigned int version)
{
    ar & pt.x;
    ar & pt.y;
}

} // namespace serialization
} // namespace boost
#endif
////////////////////////////////////////////////
// End serialization code
////////////////////////////////////////////////



namespace sim_mob {



/**
 * Simple, lightweight class to represent vector operations
 *
 * \author Seth N. Hetu
 * \author Xu Yan
 */
class DynamicVector {
private:
	DPoint pos;
	DPoint mag;

	//Helper variable; if set, it is assumed that mag==(0,0). In fact, mag remains at its previous
	// value so that scaling to zero and back produces a valid vector. The alternative is to maintain
	// the vector's angle, but this would be more costly.
	bool isZero;

#ifndef SIMMOB_DISABLE_MPI
public:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & pos;
		ar & mag;
		ar & isZero;
	}
#endif

public:
	DynamicVector(double fromX=0.0, double fromY=0.0, double toX=0.0, double toY=0.0);
	DynamicVector(const Point2D& from, const Point2D& to);
	DynamicVector(const DynamicVector& copyFrom);

	//Basic accessors
	double getX() const;   ///<Retrieve the x coordinate of the origin.
	double getY() const;   ///<Retrieve the y coordinate of the origin.

	///Retrieve the x coordinate of the origin+magnitude.
	double getEndX() const;

	///Retrieve the y coordinate of the origin+magnitude.
	double getEndY() const;

	///Retrieve the magnitude.
	double getMagnitude() const;

	//Retrieve the angle. Should still work even if the magnitude is zero
	//Returns an angle in the range 0..2*PI
	double getAngle() const;

	//Basic utility functions
	DynamicVector& translateVect(double dX, double dY);  ///<Shift this vector by dX,dY. (Moves the origin)
	DynamicVector& translateVect();  ///<Shift this vector by its own magnitude. Effectively moves the vector's origin to its "end" point.

	///Scale this vector's magnitude TO a given value. (Note that this vector need not be a unit vector.)
	DynamicVector& scaleVectTo(double val);

	//Slightly more complex
	///Flip this vector 180 degrees around the origin.
	DynamicVector& flipMirror();

	///Flip this vector 90 degrees around the origin, either clockwise or counter-clockwise.
	DynamicVector& flipNormal(bool clockwise);

	DynamicVector& flipRight();  ///<Flip this vector 90 degrees clockwise around the origin.

	DynamicVector& flipLeft();  ///<Flip this vector 90 degrees counter-clockwise around the origin.

};


}


