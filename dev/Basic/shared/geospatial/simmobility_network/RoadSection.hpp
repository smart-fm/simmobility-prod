//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>

#include "Tag.hpp"

namespace simmobility_network
{
  //Defines the various types of road sections supported by SimMobility
  enum RoadSectionType
  {
    //Default road section
    RS_TYPE_DEFAULT = 0,
    
    //Freeway
    RS_TYPE_FREEWAY = 1,
    
    //Ramp
    RS_TYPE_RAMP = 2,
    
    //Urban road
    RS_TYPE_URBANROAD = 3
  };
  
  //Defines the various categories of road sections supported by SimMobility
  enum RoadSectionCategory
  {
    //Road sections within the CBD
    RS_CATEGORY_CBD = 0,
    
    //Normal road section category
    RS_CATEGORY_DEFAULT = 1
  };

  class RoadSection
  {
  private:
    
    //Unique identifier for the road section
    unsigned int roadSectionId;
    
    //Indicates the maximum number of vehicles that the road section can accommodate
    unsigned int capacity;    
    
    //Indicates the id of the geometry information of the road section
    unsigned int geometryId;
    
    //Indicates the id of the link to which the road section belongs
    unsigned int linkId;
    
    //Indicates the maximum speed the vehicles should adhere to when travelling on the road section
    unsigned int maxSpeed;
    
    //Indicates the id for the poly-line for the road section
    unsigned int polyLineId;
    
    //The name of the road to which this section belongs
    std::string roadName;
    
    //Indicates the category of the road section
    RoadSectionCategory roadSectionCategory;
    
    //Indicates the type of the road section
    RoadSectionType roadSectionType;
    
    //Indicates the position of the road section within the link
    unsigned int sequenceNumber;
    
    //Holds additional information
    Tag *tag;

  public:
    
    RoadSection(unsigned int id, unsigned int capacity, unsigned int geometryId, unsigned int linkId, unsigned int maxSpeed, unsigned int polyLineId,
                std::string roadName, RoadSectionCategory category, RoadSectionType type, unsigned int seqNum, Tag *tag);
    
    RoadSection(const RoadSection& orig);
    
    virtual ~RoadSection();
    
    //Returns the road section id
    unsigned int getRoadSectionId() const;
    
    //Setter for the road section id
    void setRoadSectionId(unsigned int roadSectionId);
    
    //Returns the value of the road section capacity
    unsigned int getCapacity() const;
    
    //Setter for the road section capacity
    void setCapacity(unsigned int capacity);    
    
    //Returns the id of the geometry information of the road section
    unsigned int getGeometryId() const;
    
    //Setter for the id of the geometry information of the road section
    void setGeometryId(unsigned int geometryId);
    
    //Returns the id of the link to which the road section belongs
    unsigned int getLinkId() const;
    
    //Setter for the id of the link to which the road section belongs
    void setLinkId(unsigned int linkId);
    
    //Returns the max permissible speed for the road section
    unsigned int getMaxSpeed() const;
    
    //Setter for the max permissible speed for the road section
    void setMaxSpeed(unsigned int maxSpeed);
    
    //Returns the id of the poly-line for the road section
    unsigned int getPolyLineId() const;
    
    //Setter for the poly-line id for the road section
    void setPolyLineId(unsigned int polyLineId);
    
    //Returns the name of the road to which the road section belongs
    std::string getRoadName() const;
    
    //Setter for the name of the road section
    void setRoadName(std::string roadName);
    
    //Returns the category of the road section
    RoadSectionCategory getRoadSectionCategory() const;
    
    //Setter for the category of the road section
    void setRoadSectionCategory(RoadSectionCategory roadSectionCategory);
    
    //Returns the type of the road section
    RoadSectionType getRoadSectionType() const;
    
    //Setter for the type of the road section
    void setRoadSectionType(RoadSectionType roadSectionType);
    
    //Returns the sequence number of the road section within the link
    unsigned int getSequenceNumber() const;
    
    //Setter for the sequence number of the road section
    void setSequenceNumber(unsigned int sequenceNumber);
    
    //Returns a pointer to the tag which holds the additional information
    Tag* getTag() const;
    
    //Setter for the tag field which holds the additional information
    void setTag(Tag* tag);
    
  } ;
}

