//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "Node.hpp"
#include "RoadSegment.hpp"
#include "Link.hpp"

namespace simmobility_network
{
  struct WayPoint
  {

    enum
    {
      //The way point is invalid. None of the pointers are valid
      INVALID,

      //The way point is a road-segment. roadSegment points to a RoadSegment object
      NODE,

      //The way point is a road-segment. roadSegment points to a RoadSegment object
      ROAD_SEGMENT,
      
      //The way point is a link. link points to a Link object
      LINK,
      
      //The way point is a turning. turningPath points to a TurningPath object
      TURNING_PATH
    } type;

    union
    {
      const Node *node;
      const RoadSegment *roadSegment;
      const Link* link;
      const TurningPath* turningPath;
    } ;
    
    WayPoint() :
    type(INVALID), roadSegment(NULL)
    {
    }
    
    explicit WayPoint(const Node *node) :
    type(NODE), node(node)
    {
    }
    
    explicit WayPoint(const RoadSegment *segment) :
    type(ROAD_SEGMENT), roadSegment(segment)
    {
    }
    
    explicit WayPoint(const Link *link) :
	type(LINK), link(link)
	{
	}

    explicit WayPoint(const TurningPath *turningPath) : type(TURNING_PATH), turningPath(turningPath) {}

    WayPoint(const WayPoint& orig)
    {
      type = orig.type;

      switch (type) 
      {
      case INVALID:
        roadSegment = NULL;
        break;
        
      case NODE:
        node = orig.node;        
        break;
      
      case ROAD_SEGMENT:
        roadSegment = orig.roadSegment;
        break;

      case LINK:
		  link = orig.link;
		  break;

      case TURNING_PATH:
		  turningPath = orig.turningPath;
		  break;
      }
    }
    
    bool operator==(const WayPoint& rhs) const
    {
      return (type == rhs.type && node == rhs.node && roadSegment == rhs.roadSegment && link == rhs.link && turningPath == rhs.turningPath);
    }
    
    bool operator!=(const WayPoint& rhs) const
    {
      return !(*this == rhs);
    }

    WayPoint& operator=(const WayPoint& rhs)
    {
      type = rhs.type;

      switch (type) 
      {
      case INVALID:
        node = NULL;
        roadSegment = NULL;
        break;
        
      case NODE:
        node = rhs.node;
        break;
      
      case ROAD_SEGMENT:
        roadSegment = rhs.roadSegment;
        break;

      case LINK:
      		  link = rhs.link;
      		  break;

      case TURNING_PATH:
      		  turningPath = rhs.turningPath;
      		  break;
      }

      return *this;
    }
    
  } ;
}

