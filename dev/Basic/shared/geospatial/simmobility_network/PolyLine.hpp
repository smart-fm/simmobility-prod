//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <vector>

#include "Point.hpp"

namespace simmobility_network
{

  //Defines the types of poly-lines supported by SimMobility
  enum PolylineType
  {
    //Made up of straight lines connecting the points within the poly-line
    POLYLINE_TYPE_POLYPOINT = 0,

    //Bezier curve
    POLYLINE_TYPE_QUAD_BEZIER_CURVE = 1,

    //SVG
    POLYLINE_TYPE_SVGPATH = 2,

    //MITSIM format poly-line
    POLYLINE_TYPE_MITSIM = 3
  } ;

  class PolyLine
  {
  protected:

    //Unique identifier for the Poly-line
    int polyLineId;
    
    //Defines the points in the poly-line
    std::vector<Point *> points;

    //Poly-line type
    PolylineType type;

  public:
    
    PolyLine();
    
    PolyLine(const PolyLine& orig);
    
    virtual ~PolyLine();
    
    //Sets the id of the poly-line
    void setPolyLineId(int polyLineId);
    
    //Returns the id of the poly-line
    int getPolyLineId() const;
    
    //Sets the points within the poly-line
    void setPoints(std::vector<Point*>& points);
    
    //Returns the vector of points within the poly-line
    const std::vector<Point*>& getPoints() const;
    
    //Sets the type of the poly-line
    void setType(PolylineType type);
    
    //Returns the type of the poly-line
    PolylineType getType() const;

  } ;
}