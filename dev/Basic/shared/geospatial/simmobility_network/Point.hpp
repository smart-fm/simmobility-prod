//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace simmobility_network
{

  class Point
  {
  private:
    
    //Indicates the position of the point in a line
    unsigned int sequenceNumber;
    
    //X coordinate
    double x;
    
    //Y coordinate
    double y;
    
    //Z coordinate
    double z;
    
  public:
    
    Point();
    
    Point(const Point& orig);
    
    virtual ~Point();
    
    //Returns the sequence number of the point within a line
    unsigned int getSequenceNumber() const;
    
    //Sets the sequence number of the point within a line
    void setSequenceNumber(unsigned int sequenceNumber);
    
    //Returns the x coordinate
    double getX() const;
    
    //Sets the x coordinate
    void setX(double x);
    
    //Returns the y coordinate
    double getY() const;
    
    //Sets the y coordinate
    void setY(double y);
    
    //Returns the z coordinate
    double getZ() const;
    
    //Sets the z coordinate
    void setZ(double z);
    
  } ;
}