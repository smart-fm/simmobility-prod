//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>

#include "Tag.hpp"
#include "PolyLine.hpp"
#include "LaneConnector.hpp"

namespace simmobility_network
{
  //Defines the lane rules
  enum LaneRules
  {
    LANE_RULES_IS_PEDESTRIAN_LANE
  };

  class Lane
  {
  private:
    
    //Unique identifier for the lane
    unsigned int laneId;
    
    //The outgoing lane connectors
    std::vector<LaneConnector *> laneConnectors;
    
    //Indicates the index of the lane
    unsigned int laneIndex;
    
    //Indicates the lane rules
    LaneRules laneRules;
    
    //Represents the poly-line of the lane
    PolyLine *polyLine;
    
    //Holds additional information
    std::vector<Tag> tags;
    
    //The width of the lane
    double width;
    
  public:
    
    Lane();
    
    Lane(const Lane& orig);
    
    virtual ~Lane();
    
    //Returns the lane id
    unsigned int getLaneId() const;
    
    //Sets the lane id
    void setLaneId(unsigned int laneId);
    
    //Returns the outgoing lane connectors
    const std::vector<LaneConnector*>& getLaneConnectors() const;
    
    //Sets the outgoing lane connectors
    void setLaneConnectors(std::vector<LaneConnector*>& laneConnectors);
    
    //Returns the lane index
    unsigned int getLaneIndex() const;
    
    //Sets the lane index
    void setLaneIndex(unsigned int laneIndex);
    
    //Returns the poly-line for the lane
    PolyLine* getPolyLine() const;
    
    //Sets the poly-line for the name
    void setPolyLine(PolyLine* polyLine);    
    
    //Returns a vector of tags which holds the additional information
    const std::vector<Tag>& getTags() const;
    
    //Setter for the tags field which holds the additional information
    void setTags(std::vector<Tag>& tags);
    
    //Returns the width of the lane
    double getWidth() const;  
    
    //Sets the lane width
    void setWidth(double width);    
    
  } ;
}
