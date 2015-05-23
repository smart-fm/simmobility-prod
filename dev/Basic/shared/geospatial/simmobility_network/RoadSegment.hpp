//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <vector>

#include "Lane.hpp"
#include "PolyLine.hpp"
#include "Tag.hpp"

namespace simmobility_network
{
  //Defines the various types of road segments supported by SimMobility
  enum RoadSegmentType
  {
    //Default road segment
    RS_TYPE_DEFAULT = 0,
    
    //Freeway
    RS_TYPE_FREEWAY = 1,
    
    //Ramp
    RS_TYPE_RAMP = 2,
    
    //Urban road
    RS_TYPE_URBANROAD = 3
  };
  
  //Defines the various categories of road segments supported by SimMobility
  enum RoadSegmentCategory
  {
    //Road segments within the CBD
    RS_CATEGORY_CBD = 0,
    
    //Normal road segment category
    RS_CATEGORY_DEFAULT = 1
  };

  class RoadSegment
  {
  private:
    
    //Unique identifier for the road segment
    unsigned int roadSegmentId;
    
    //Indicates the maximum number of vehicles that the road segment can accommodate
    unsigned int capacity;
    
    //The lanes that make up th road segment
    std::vector<Lane *> lanes;
    
    //Indicates the maximum speed the vehicles should adhere to when travelling on the road segment
    unsigned int maxSpeed;
    
    //Represents the poly-line for the road segment
    PolyLine *polyLine;
    
    //The name of the road to which this segment belongs
    std::string roadName;
    
    //Indicates the position of the road segment within the link
    unsigned int sequenceNumber;
    
    //Holds additional information
    std::vector<Tag> tags;

  public:
    
    RoadSegment();
    
    RoadSegment(const RoadSegment& orig);
    
    virtual ~RoadSegment();
    
    //Returns the road segment id
    unsigned int getRoadSectionId() const;
    
    //Setter for the road segment id
    void setRoadSectionId(unsigned int roadSectionId);
    
    //Returns the value of the road segment capacity
    unsigned int getCapacity() const;
    
    //Setter for the road segment capacity
    void setCapacity(unsigned int capacity);
    
    //Returns the vector of lanes that make up the segment
    const std::vector<Lane*>& getLanes() const;
    
    //Sets the vector of lanes that make up the segment
    void setLanes(std::vector<Lane*>& lanes);    
    
    //Returns the id of the link to which the road segment belongs
    unsigned int getLinkId() const;
    
    //Setter for the id of the link to which the road segment belongs
    void setLinkId(unsigned int linkId);
    
    //Returns the max permissible speed for the road segment
    unsigned int getMaxSpeed() const;
    
    //Setter for the max permissible speed for the road segment
    void setMaxSpeed(unsigned int maxSpeed);
    
    //Returns the poly-line for the road segment
    PolyLine* getPolyLine() const;
    
    //Sets the poly-line for the road segment
    void setPolyLine(PolyLine *polyLine);
    
    //Returns the name of the road to which the road segment belongs
    std::string getRoadName() const;
    
    //Setter for the name of the road segment
    void setRoadName(std::string roadName);
    
    //Returns the category of the road segment
    RoadSegmentCategory getRoadSectionCategory() const;
    
    //Setter for the category of the road segment
    void setRoadSectionCategory(RoadSegmentCategory roadSectionCategory);
    
    //Returns the type of the road segment
    RoadSegmentType getRoadSectionType() const;
    
    //Setter for the type of the road segment
    void setRoadSectionType(RoadSegmentType roadSectionType);
    
    //Returns the sequence number of the road segment within the link
    unsigned int getSequenceNumber() const;
    
    //Setter for the sequence number of the road segment
    void setSequenceNumber(unsigned int sequenceNumber);
    
    //Returns a vector of tags which holds the additional information
    const std::vector<Tag>& getTags() const;
    
    //Setter for the tags field which holds the additional information
    void setTag(std::vector<Tag>& tags);
    
  } ;
}

