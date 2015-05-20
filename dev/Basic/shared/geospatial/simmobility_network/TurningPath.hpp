//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace simmobility_network
{

  class TurningPath
  {
  private:
    
    //Unique identifier for the turning path
    unsigned int turningPathId;
    
    //Indicates the id of the lane where the turning path begins
    unsigned int fromLaneId;
    
    //Indicates the id of the geometry information for the turning path
    unsigned int geometryId;
    
    //Indicates the id of the poly-line for the turning path
    unsigned int polyLineId;
    
    //Holds the additional information
    Tag *tag;
    
    //Indicates the id of the lane at which the turning path ends
    unsigned int toLaneId;
    
    //Indicates the id of the turning group to which this turning path belongs
    unsigned int turningGroupId;
    
  public:
    
    TurningPath(unsigned int id, unsigned int fromLane, unsigned int geometryId, unsigned int polyLineId, Tag *tag, unsigned int toLane,
                unsigned int turningGroup);
    
    TurningPath(const TurningPath& orig);
    
    virtual ~TurningPath();
    
    //Returns the id of the turning path
    unsigned int getTurningPathId() const;
    
    //Setter for the turning path id
    void setTurningPathId(unsigned int turningPathId);
    
    //Returns the id of the lane from which this turning path originates
    unsigned int getFromLaneId() const;
    
    //Setter for the id of the lane from which this turning path originates
    void setFromLaneId(unsigned int fromLaneId);
    
    //Returns the geometry id of the turning path
    unsigned int getGeometryId() const;
    
    //Setter for the geometry id
    void setGeometryId(unsigned int geometryId);
    
    //Returns the id of the poly-line for the turning path
    unsigned int getPolyLineId() const;
    
    //Setter for the id of the poly-line of the turning path
    void setPolyLineId(unsigned int polyLineId);
    
    //Returns a pointer to the tag which holds the additional information
    Tag* getTag() const;
    
    //Setter for the tag field which holds the additional information
    void setTag(Tag* tag);
    
    //Returns the id of the lane where the turning path ends
    unsigned int getToLaneId() const;
    
    //Setter for the id of the lane where the turning path ends
    void setToLaneId(unsigned int toLaneId);
    
    //Returns the id of the turning group of which this turning path is a part
    unsigned int getTurningGroupId() const;
    
    //Setter for the id of the turning group of which this turning path is a part
    void setTurningGroupId(unsigned int turningGroupId);

  } ;
}


