//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>

#include "Tag.hpp"

namespace simmobility_network
{

  class Link
  {
  private:
    
    //Unique identifier for the link
    unsigned int linkId;
    
    //Indicates the node from which this link begins
    unsigned int fromNodeId;
    
    //The identifier for the geometry information of the link
    unsigned int geometryId;
    
    //The identifier for the poly-line of the link
    unsigned int polyLineId;
    
    //The name of the road this link represents
    std::string roadName;
    
    //Holds the additional information
    Tag *tag;
    
    //Indicates the node at which this link ends
    unsigned int toNodeId;
    
  public:
    
    Link(unsigned int id, unsigned int fromNode, unsigned int geometryId, unsigned int polyLineId, std::string roadName, 
         Tag *tag, unsigned int toNode);
    
    Link(const Link& orig);
    
    virtual ~Link();
    
    //Returns the link id
    unsigned int getLinkId() const;
    
    //Setter for the link id
    void setLinkId(unsigned int linkId);
    
    //Returns the id of the node from where the link begins
    unsigned int getFromNodeId() const;
    
    //Setter for the from node id
    void setFromNodeId(unsigned int fromNodeId);
    
    //Returns the id of the geometry information
    unsigned int getGeometryId() const;
    
    //Setter for the geometry id
    void setGeometryId(unsigned int geometryId);
    
    //Returns the id of the poly-line
    unsigned int getPolyLineId() const;
    
    //Setter for the poly-line id
    void setPolyLineId(unsigned int polyLineId);
    
    //Returns the road name
    std::string getRoadName() const;
    
    //Setter for the road name
    void setRoadName(std::string roadName);
    
    //Returns a pointer to the tag which holds the additional information
    Tag* getTag() const;
    
    //Setter for the tag field which holds the additional information
    void setTag(Tag* tag);
    
    unsigned int getToNodeId() const;
    
    void setToNodeId(unsigned int toNodeId);
  } ;
}

