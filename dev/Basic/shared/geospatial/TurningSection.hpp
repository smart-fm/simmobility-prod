//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "geospatial/Lane.hpp"

namespace sim_mob
{

  class TurningSection
  {
  public:
    TurningSection();
    TurningSection(const TurningSection & ts);

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

  public:
    std::string sectionId;
    // TODO phases

    /// polyline points
    std::vector<Point2D> polylinePoints;

    sim_mob::Lane* laneFrom;
    sim_mob::Lane* laneTo;

  public:
    std::vector<TurningSection*>& getConflictTurnings();
  public:
    std::vector<TurningSection*> confilicts;
  } ;

};// end namespace
