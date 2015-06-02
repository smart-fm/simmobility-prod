//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

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
      ROAD_SEGMENT,
	  LINK
    } type;

    union
    {
      RoadSegment *roadSegment;
      Link* link;
    } ;
    
    WayPoint() :
    type(INVALID), roadSegment(NULL)
    {
    }
    
    explicit WayPoint(RoadSegment * segment) :
    type(ROAD_SEGMENT), roadSegment(segment)
    {
    }
    
    explicit WayPoint(Link* link) :
	type(LINK), link(link)
	{
	}

    WayPoint(const WayPoint& orig)
    {
      type = orig.type;

      switch (type) 
      {
      case INVALID:
        roadSegment = NULL;
        break;
      
      case ROAD_SEGMENT:
        roadSegment = orig.roadSegment;
        break;

      case LINK:
		  link = orig.link;
		  break;
      }
    }
    
    bool operator==(const WayPoint& rhs) const
    {
      return (type == rhs.type && roadSegment == rhs.roadSegment && link == rhs.link);
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
        roadSegment = NULL;
        break;
      
      case ROAD_SEGMENT:
        roadSegment = rhs.roadSegment;
        break;

      case LINK:
      		  link = rhs.link;
      		  break;
      }

      return *this;
    }
    
  } ;
}

