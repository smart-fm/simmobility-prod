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

  //Defines the rules for the bus lanes
  enum BusLaneRules
  {
    //Both cars and buses can use the lane during the entire day
    BUS_LANE_RULES_CAR_AND_BUS = 0,
    
    //Buses only from Mon-Fri: 0730-0930 and 1700-2000
    BUS_LANE_RULES_NORMAL_BUS_LANE = 1,
    
    //Buses only from Mon-Sat: 0730-2000
    BUS_LANE_RULES_FULL_DAY_BUS_LANE = 2
  };

  class Lane
  {
  private:
    
    //Unique identifier for the lane
    unsigned int laneId;
    
    //Identifies the rule applicable for the bus lane
    BusLaneRules busLaneRules;
    
    //Defines if a vehicle can park on the lane
    bool canVehiclePark;
    
    //Defines if a vehicle can stop on the lane
    bool canVehicleStop;
    
    //Defines whether the lane has a road shoulder
    bool hasRoadShoulder;
    
    //Defines whether a high occupancy vehicle is allowed on the lane
    bool isHOV_Allowed;
    
    //The outgoing lane connectors
    std::vector<LaneConnector *> laneConnectors;
    
    //Indicates the index of the lane
    unsigned int laneIndex;
    
    //Represents the poly-line of the lane
    PolyLine *polyLine;
    
    //Holds additional information
    std::vector<Tag> tags;
    
    //Defines the types of vehicles that can use the lane
    //7 bits are used to identify the modes as follows:
    //Pedestrian Bicycle Car Van Truck Bus Taxi
    unsigned int vehicleMode;
    
    //The width of the lane
    double width;
    
  public:
    
    Lane();
    
    Lane(const Lane& orig);
    
    virtual ~Lane();
    
    //Returns the lane id
    unsigned int getLaneId() const;

    //Returns the bus lane rules
    BusLaneRules getBusLaneRules() const;

    //Sets the bus lane rules
    void setBusLaneRules(BusLaneRules busLaneRules);

    //Returns true if parking is allowed on the lane, else returns false
    bool isParkingAllowed() const;

    //Sets whether vehicle parking is allowed
    void setCanVehiclePark(bool canVehiclePark);
    
    //Returns true if stopping is allowed on the lane, else returns false
    bool isStoppingAllowed() const;
    
    //Sets whether stopping on the lane is allowed
    void setCanVehicleStop(bool canVehicleStop);
    
    //Returns true if the road shoulder is present, else returns false
    bool doesLaneHaveRoadShoulder() const;
    
    //Sets whether the road shoulder is present
    void setHasRoadShoulder(bool hasRoadShoulder);
    
    //Returns true if a high occupancy vehicle is allowed on the lane, else returns false
    bool isHighOccupancyVehicleAllowed() const;
    
    //Sets whether a high occupancy vehicle is allowed on the lane
    void setHighOccupancyVehicleAllowed(bool HighOccupancyVehicleAllowed);

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
