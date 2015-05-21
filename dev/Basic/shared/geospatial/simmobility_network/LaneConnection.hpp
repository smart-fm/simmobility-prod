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
    
    unsigned int getLaneConnectionId() const;
    
    void setLaneConnectionId(unsigned int laneConnectionId);
    
    unsigned int getFromLaneId() const;
    
    void setFromLaneId(unsigned int fromLaneId);
    
    unsigned int getFromRoadSectionId() const;
    
    void setFromRoadSectionId(unsigned int fromRoadSectionId);
    
    Tag* getTag() const;
    
    void setTag(Tag* tag);
    
    unsigned int getToLaneId() const;
    
    void setToLaneId(unsigned int toLaneId);
    
    unsigned int getToRoadSectionId() const;
    
    void setToRoadSectionId(unsigned int toRoadSectionId);

    } ;
}

