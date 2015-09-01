//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace simmobility_network
{

  class Point
  {
  private:
    
    //Indicates the poly-line to which the point belongs
    unsigned int polyLineId;
    
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
    
    Point(double x, double y);
    
    Point(unsigned int id, unsigned int seqNum, double x, double y, double z);
    
    Point(const Point& orig);
    
    virtual ~Point();
    
    //Returns the id of the poly-line to which the point belongs
    unsigned int getPolyLineId() const;
    
    //Sets the id of the poly-line to which the point belongs
    void setPolyLineId(unsigned int polyLineId);
    
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