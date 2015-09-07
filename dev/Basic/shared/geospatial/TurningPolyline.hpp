//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#ifndef GEOSPATIAL_TURNINGPOLYLINE_H_
#define GEOSPATIAL_TURNINGPOLYLINE_H_

#include "geospatial/Polyline.hpp"
#include "geospatial/Polypoint.hpp"
#include "geospatial/TurningSection.hpp"

namespace sim_mob {

class TurningSection;
class Polyline;

  class TurningPolyline : public Polyline
  {
  private:
    
    //Id of the turning to which the poly-line belongs
    int turningId;
    
    //The Turning object to which the poly-line belongs
    TurningSection *turning;
    
    //The points in the poly-line
    std::vector<Polypoint*> polypoints;
    
  public:
    
    TurningPolyline();
    
    TurningPolyline(const TurningPolyline& tp);
    
    virtual ~TurningPolyline();
    
    //Setter for the poly-points
    void setPolypoints(std::vector<Polypoint*> polypoints);
    
    //Adds a poly-point to the poly-line
    void addPolypoint(Polypoint *point);
    
    //Getter for the poly-points
    std::vector<Polypoint*> getPolypoints() const;
    
    //Setter for the turning
    void setTurning(TurningSection* turning);
    
    //Getter for the turning
    TurningSection* getTurning() const;
    
    //Setter for the turning id
    void setTurningId(int turningId);
    
    //Getter for the turning id
    int getTurningId() const;
  } ;

} /* namespace sim_mob */

#endif /* GEOSPATIAL_TURNINGPOLYLINE_H_ */
