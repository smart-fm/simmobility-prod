//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>

#include "Tag.hpp"

namespace simmobility_network
{
  //Defines the various types of nodes supported by the SimMobility network
  enum NodeType
  {
    //Default node
    DEFAULT_NODE = 0,
    
    //This type of node only has incoming road sections connected to it
    SINK_NODE = 1,
    
    //This type of node only has outgoing road sections connected to it
    SOURCE_NODE = 2,
    
    //This type of node is an intersection with or without a traffic signal
    INTERSECTION_NODE = 3,
    
    //This type of node is a merging node
    MERGE_NODE = 4
  };

  class Node
  {
  private:
    
    //The unique identifier for a Node
    unsigned int nodeId;
    
    //The type of the node
    NodeType nodeType;
    
    //Holds additional information
    Tag *tag;
    
    //The identifier for the traffic light if present at the node
    unsigned int trafficLightId;

  public:
    
    Node(unsigned int id, NodeType type, Tag *tag, unsigned int trafficLightId);
    
    Node(const Node& orig);
    
    virtual ~Node();
    
    //Sets the node id
    void setNodeId(unsigned int nodeId);
    
    //Returns the node id
    unsigned int getNodeId() const;
    
    //Sets the node type
    void setNodeType(NodeType nodeType);
    
    //Returns the type of the node
    NodeType getNodeType() const;
    
    //Setter for the tag field which holds the additional information
    void setTag(Tag *tag);
    
    //Returns a pointer to the tag which holds the additional information
    Tag* getTag() const;
    
    //Sets the traffic light id
    void setTrafficLightId(unsigned int trafficLightId);
    
    //Returns the traffic light id
    unsigned int getTrafficLightId() const;

  } ;
}
