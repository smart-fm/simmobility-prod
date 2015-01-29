//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "geospatial/Lane.hpp"
#include "TurningConflict.hpp"

namespace sim_mob
{
class TurningConflict;

  class TurningSection
  {

  public:
    int dbId;
    double from_xpos;
    double from_ypos;
    double to_xpos;
    double to_ypos;
    std::string from_road_section;
    std::string to_road_section;
    int from_lane_index;
    int to_lane_index;

    std::string sectionId;
    // TODO phases

    sim_mob::RoadSegment* fromSeg;
    sim_mob::RoadSegment* toSeg;

    /// polyline points
    std::vector<Point2D> polylinePoints;

    const sim_mob::Lane* laneFrom;
    const sim_mob::Lane* laneTo;
    
    std::vector<TurningSection*> conflictingTurningSections;
    std::vector<TurningConflict* > turningConflicts;

  public:
    TurningSection();
    TurningSection(const TurningSection & ts);
    
    std::vector<TurningSection*>& getConflictTurningSections();
  
    TurningConflict* getTurningConflict(const TurningSection* ts);
  } ;

};// end namespace
