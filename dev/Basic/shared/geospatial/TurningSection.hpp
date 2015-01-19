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

  public:
    std::string sectiongId;
    // phases
    std::vector<TurningSection*> confilicts;

    /// polyline points
    std::vector<Point2D> polylinePoints;

    sim_mob::Lane* laneFrom;
    sim_mob::Lane* laneTo;
  } ;

};// end namespace
