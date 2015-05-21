//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "Tag.hpp"

namespace simmobility_network
{

  class LaneConnection
  {
  private:
    
    //Unique identifier for the lane connection
    unsigned int laneConnectionId;
    
    //Indicates the id of the lane from which the lane connection originates
    unsigned int fromLaneId;
    
    //Indicates the id of the road section from which the lane connection originates
    unsigned int fromRoadSectionId;
    
    //Holds additional information
    Tag *tag;
    
    //Indicates the id of the lane at which the lane connection terminates
    unsigned int toLaneId;
    
    //Indicates the id of the road section at which the lane connection terminates
    unsigned int toRoadSectionId;    

  public:

    LaneConnection(unsigned int id, unsigned int fromLane, unsigned int fromRoadSection, Tag *tag, 
                   unsigned int toLane, unsigned int toRoadSection);

    LaneConnection(const LaneConnection& orig);

    virtual ~LaneConnection();
    
    //Returns the id of the lane connection
    unsigned int getLaneConnectionId() const;
    
    //Sets the lane connection id
    void setLaneConnectionId(unsigned int laneConnectionId);
    
    //Returns the id of the lane from which the lane connection begins
    unsigned int getFromLaneId() const;
    
    //Sets the id of the lane from which the lane connection begins
    void setFromLaneId(unsigned int fromLaneId);
    
    //Returns the id of the road section from which the lane connection begins
    unsigned int getFromRoadSectionId() const;
    
    //Sets the id of the road section from which the lane connection begins
    void setFromRoadSectionId(unsigned int fromRoadSectionId);
    
    //Returns a pointer to the tag which holds the additional information
    Tag* getTag() const;
    
    //Setter for the tag field which holds the additional information
    void setTag(Tag* tag);
    
    //Returns the id of the lane at which the lane connection ends
    unsigned int getToLaneId() const;
    
    //Sets the id of the lane at which the lane connection ends
    void setToLaneId(unsigned int toLaneId);
    
    //Returns the id of the road section at which the lane connection ends
    unsigned int getToRoadSectionId() const;
    
    //Sets the id of the road section at which the lane connection ends
    void setToRoadSectionId(unsigned int toRoadSectionId);

    } ;
}

