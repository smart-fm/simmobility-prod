//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <vector>

#include "PolyLine.hpp"
#include "Tag.hpp"

namespace simmobility_network
{
  //Defines the various categories of links supported by SimMobility
  enum LinkCategory
  {
    //The default category
    LINK_CATEGORY_DEFAULT = 0
  };

  class Link
  {
  private:
    
    //Unique identifier for the link
    unsigned int linkId;
    
    //Indicates the node from which this link begins
    unsigned int fromNodeId;
    
    //Indicates the link category
    LinkCategory linkCategory;
    
    //The name of the road this link represents
    std::string roadName;
    
    //Holds the additional information
    std::vector<Tag> tags;
    
    //Indicates the node at which this link ends
    unsigned int toNodeId;
    
  public:
    
    Link();
    
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
    
    //Returns the link category
    LinkCategory getLinkCategory() const;
    
    //Sets the link category
    void setLinkCategory(LinkCategory linkCategory);
    
    //Returns the road name
    std::string getRoadName() const;
    
    //Setter for the road name
    void setRoadName(std::string roadName);
    
    //Returns a vector of tags which holds the additional information
    const std::vector<Tag>& getTags() const;
    
    //Setter for the tags field which holds the additional information
    void setTags(std::vector<Tag>& tags);
    
    unsigned int getToNodeId() const;
    
    void setToNodeId(unsigned int toNodeId);
  } ;
}

