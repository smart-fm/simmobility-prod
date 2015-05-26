//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>

#include "Link.hpp"
#include "Node.hpp"
#include "Point.hpp"
#include "PolyLine.hpp"
#include "TurningGroup.hpp"
#include "TurningPath.hpp"

namespace simmobility_network
{
  
  class RoadNetwork
  {
  private:
    
    //This map stores all the links in the network, with the Link id as the key for retrieval
    std::map<unsigned int, Link *> mapOfIdVsLinks;
    
    //This map stores all the nodes in the network, with the Node id as the key for retrieval
    std::map<unsigned int, Node *> mapOfIdvsNodes;
    
    //This map stores all the turning groups in the network, with the group id as the key for retrieval
    std::map<unsigned int, TurningGroup *> mapOfIdvsTurningGroups;
    
    //This map stores all the turning paths in the network, with the turning id as the key for retrieval
    std::map<unsigned int, TurningPath *> mapOfIdvsTurningPaths;
    
    //This map stores all the turning poly-lines in the network with the poly-line id as the key for retrieval
    std::map<unsigned int, PolyLine *> mapOfIdVsTurningPolyLines;
    
  public:
    
    RoadNetwork();
    
    virtual ~RoadNetwork();
    
    const std::map<unsigned int, Link*>& getMapOfIdVsLinks() const;

    const std::map<unsigned int, Node*>& getMapOfIdvsNodes() const;
    
    const std::map<unsigned int, TurningGroup*>& getMapOfIdvsTurningGroups() const;
    
    const std::map<unsigned int, TurningPath*>& getMapOfIdvsTurningPaths() const;
    
    const std::map<unsigned int, PolyLine*>& getMapOfIdVsTurningPolyLines() const;

    void addLink(Link *link);

    void addNode(Node *node);
    
    void addTurningConflict(TurningConflict *turningConflict);
        
    void addTurningGroup(TurningGroup *turningGroup);
    
    void addTurningPath(TurningPath *turningPath);
    
    void addTurningPolyLine(Point *point);
    
  } ;
}

