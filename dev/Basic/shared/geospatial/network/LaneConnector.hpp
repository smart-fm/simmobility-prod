//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace simmobility_network
{

  class LaneConnector
  {
  private:
    
    //Unique identifier for the lane connection
    unsigned int laneConnectionId;
    
    //Indicates the id of the lane from which the lane connection originates
    unsigned int fromLaneId;
    
    //Indicates the id of the road segment from which the lane connection originates
    unsigned int fromRoadSegmentId;
    
    //Indicates the id of the lane at which the lane connection terminates
    unsigned int toLaneId;
    
    //Indicates the id of the road segment at which the lane connection terminates
    unsigned int toRoadSegmentId;    

  public:

    LaneConnector();

    LaneConnector(const LaneConnector& orig);

    virtual ~LaneConnector();
    
    //Returns the id of the lane connection
    unsigned int getLaneConnectionId() const;
    
    //Sets the lane connection id
    void setLaneConnectionId(unsigned int laneConnectionId);
    
    //Returns the id of the lane from which the lane connection begins
    unsigned int getFromLaneId() const;
    
    //Sets the id of the lane from which the lane connection begins
    void setFromLaneId(unsigned int fromLaneId);
    
    //Returns the id of the road segment from which the lane connection begins
    unsigned int getFromRoadSegmentId() const;
    
    //Sets the id of the road segment from which the lane connection begins
    void setFromRoadSegmentId(unsigned int fromRoadSectionId);
    
    //Returns the id of the lane at which the lane connection ends
    unsigned int getToLaneId() const;
    
    //Sets the id of the lane at which the lane connection ends
    void setToLaneId(unsigned int toLaneId);
    
    //Returns the id of the road section at which the lane connection ends
    unsigned int getToRoadSegmentId() const;
    
    //Sets the id of the road section at which the lane connection ends
    void setToRoadSegmentId(unsigned int toRoadSectionId);

    } ;
}

