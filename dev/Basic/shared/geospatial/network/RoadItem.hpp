//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "Tag.hpp"

namespace simmobility_network
{

  class RoadItem
  {
  private:
    
    //Unique identifier for the road item
    unsigned int roadItemId;
    
    //Indicates the id of the geometry information
    unsigned int geometryId;
    
    //Indicates the id of the poly-line for the road item
    unsigned int polyLineId;
    
    //Indicates the id of the road section to which the road item belongs
    unsigned int roadSectionId;
    
    //Holds additional information
    Tag *tag;
    
  public:
    
    RoadItem(unsigned int id, unsigned int geomteryId, unsigned int polyLineId, unsigned int roadSectionId, Tag *tag);
    
    RoadItem(const RoadItem& orig);
    
    virtual ~RoadItem();
    
    //Returns the id of the road item
    unsigned int getRoadItemId() const;
    
    //Sets the id of the road item
    void setRoadItemId(unsigned int roadItemId);
    
    //Returns the id of the geometry information
    unsigned int getGeometryId() const;
    
    //Sets the id of the geometry information
    void setGeometryId(unsigned int geometryId);
    
    //Returns the poly-line id
    unsigned int getPolyLineId() const;
    
    //Sets the poly-line id
    void setPolyLineId(unsigned int polyLineId);
    
    //Returns the id of the road section to which the road item belongs
    unsigned int getRoadSectionId() const;
    
    //Sets the id of the road section to which the road item belongs
    void setRoadSectionId(unsigned int roadSectionId);
    
    //Returns a pointer to the tag which holds the additional information
    Tag* getTag() const;
    
    //Setter for the tag field which holds the additional information
    void setTag(Tag* tag);

  } ;
}

