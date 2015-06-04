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
    
    //This map stores all the lanes in the network, with the lane id as the key for retrieval
    std::map<unsigned int, Lane *> mapOfIdVsLanes;
    
    //This map stores all the segments in the network, with the segment id as the key for retrieval
    std::map<unsigned int, RoadSegment *> mapOfIdVsRoadSegments;
    
    //This map stores all the turning conflicts in the network, with the conflict id as the key for retrieval
    std::map<unsigned int, TurningConflict *> mapOfIdVsTurningConflicts;
    
    //This map stores all the turning groups in the network, with the group id as the key for retrieval
    std::map<unsigned int, TurningGroup *> mapOfIdvsTurningGroups;
    
    //This map stores all the turning paths in the network, with the turning id as the key for retrieval
    std::map<unsigned int, TurningPath *> mapOfIdvsTurningPaths;
    
  public:
    
    RoadNetwork();
    
    virtual ~RoadNetwork();
    
    //Returns the map of link id vs links
    const std::map<unsigned int, Link*>& getMapOfIdVsLinks() const;

    //Returns the map of node id vs nodes
    const std::map<unsigned int, Node*>& getMapOfIdvsNodes() const;
    
    //Returns the map of turning group id vs turning groups
    const std::map<unsigned int, TurningGroup*>& getMapOfIdvsTurningGroups() const;
    
    //Returns the map of turning path id vs turning path
    const std::map<unsigned int, TurningPath*>& getMapOfIdvsTurningPaths() const;
    
    //Adds a lane to the road network
    void addLane(Lane *lane);
    
    //Adds a lane connector to the road network
    void addLaneConnector(LaneConnector *connector);
    
    //Adds a lane poly-line to the road network
    void addLanePolyLine(Point point);

    //Adds a link to the road network
    void addLink(Link *link);

    //Adds a node to the road network
    void addNode(Node *node);
    
    //Adds a road segment to the road network
    void addRoadSegment(RoadSegment *segment);
    
    //Adds a segment poly-line to the road network
    void addSegmentPolyLine(Point point);
    
    //Adds a turning conflict to the road network
    void addTurningConflict(TurningConflict *turningConflict);

    //Adds a turning group to the road network
    void addTurningGroup(TurningGroup *turningGroup);
    
    ////Adds a turning path to the road network
    void addTurningPath(TurningPath *turningPath);
    
    //Adds a turning poly-line to the road network
    void addTurningPolyLine(Point point);
    
    //Looks for the required node from the map of nodes and returns a pointer to it, if found
    Node *getNodeById(unsigned int nodeId);
    
  } ;
}

