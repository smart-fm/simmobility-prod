//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>

#include "Link.hpp"
#include "Node.hpp"

namespace simmobility_network
{
  
  class RoadNetwork
  {
  private:
    
    //This map stores all the nodes in the network, with the Node id as the key for retrieval
    std::map<unsigned int, Node *> mapOfIdvsNodes;
    
    //This map stores all the links in the network, with the Link id as the key for retrieval
    std::map<unsigned int, Link *> mapOfIdVsLinks;
    
  public:
    
    RoadNetwork();
    
    RoadNetwork(const RoadNetwork& orig);
    
    virtual ~RoadNetwork();
    
    void setMapOfIdvsNodes(std::map<unsigned int, Node*>& mapOfIdvsNodes);
    
    const std::map<unsigned int, Node*>& getMapOfIdvsNodes() const;
    
    void setMapOfIdVsLinks(std::map<unsigned int, Link*>& mapOfIdVsLinks);
    
    const std::map<unsigned int, Link*>& getMapOfIdVsLinks() const;

  } ;
}

