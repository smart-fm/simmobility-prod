//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <cmath>
#include <stdexcept>
#include <iostream>
#include <boost/thread.hpp>

#include "conf/settings/DisableMPI.h"
#include "geospatial/network/Point.hpp"
#include "logging/Log.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#endif


////////////////////////////////////////////////
// Serialization for Point. This is how one would
// serialize *outside* the class itself. You can also
// put the serialize() function inside Point; see
// FixedDelayed<> for an example of this alternate approach.
////////////////////////////////////////////////
#ifndef SIMMOB_DISABLE_MPI
namespace boost
{
  namespace serialization
  {
    template<class Archive>
    void serialize(Archive & ar, sim_mob::Point& pt, const unsigned int version)
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

namespace sim_mob 
{
  /**
   * Simple, lightweight class to represent vector operations
   *
   * \author Seth N. Hetu
   * \author Xu Yan
   */
  class DynamicVector
  {
  private:
    Point pos;
    Point mag;

    //Helper variable; if set, it is assumed that mag==(0,0). In fact, mag remains at its previous
    //value so that scaling to zero and back produces a valid vector. The alternative is to maintain
    //the vector's angle, but this would be more costly.
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
    DynamicVector(double fromX = 0.0, double fromY = 0.0, double toX = 0.0, double toY = 0.0);
    DynamicVector(const Point& from, const Point& to);
    DynamicVector(const DynamicVector& copyFrom);

    //Retrieve the x coordinate of the origin.
    double getX() const;
    
    //Retrieve the y coordinate of the origin.
    double getY() const;

    //Retrieve the x coordinate of the origin+magnitude.
    double getEndX() const;

    //Retrieve the y coordinate of the origin+magnitude.
    double getEndY() const;

    //Retrieve the magnitude.
    double getMagnitude() const;

    //Retrieve the angle. Should still work even if the magnitude is zero
    //Returns an angle in the range 0..2*PI
    double getAngle() const;

    //Shift this vector by dX,dY. (Moves the origin)
    DynamicVector& translateVect(double dX, double dY);
    
    //Shift this vector by its own magnitude. Effectively moves the vector's origin to its "end" point.
    DynamicVector& translateVect();  

    ///Scale this vector's magnitude TO a given value. (Note that this vector need not be a unit vector.)
    DynamicVector& scaleVectTo(double val);

    //Flip this vector 180 degrees around the origin.
    DynamicVector& flipMirror();

    //Flip this vector 90 degrees around the origin, either clockwise or counter-clockwise.
    DynamicVector& flipNormal(bool clockwise);

    //Flip this vector 90 degrees clockwise around the origin.
    DynamicVector& flipRight();

    //Flip this vector 90 degrees counter-clockwise around the origin.
    DynamicVector& flipLeft();

  } ;
}

