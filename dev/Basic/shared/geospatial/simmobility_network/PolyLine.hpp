//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <vector>

#include "Point.hpp"

namespace simmobility_network
{

  class PolyLine
  {
  protected:

    //Unique identifier for the Poly-line
    int polyLineId;
    
    //Defines the points in the poly-line
    std::vector<Point> points;

  public:
    
    PolyLine();
    
    PolyLine(const PolyLine& orig);
    
    virtual ~PolyLine();
    
    //Sets the id of the poly-line
    void setPolyLineId(int polyLineId);
    
    //Returns the id of the poly-line
    int getPolyLineId() const;
    
    //Returns the vector of points within the poly-line
    const std::vector<Point>& getPoints() const;
    
    Point getFirstPoint() {return points[0];}

    Point getLastPoint() {return points.at(points.size()-1);}

    //Adds a point to the poly-line
    void addPoint(Point point);

    double length;

  } ;
}
