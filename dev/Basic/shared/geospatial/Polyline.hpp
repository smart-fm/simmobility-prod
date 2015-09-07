//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#ifndef GEOSPATIAL_POLYLINE_H_
#define GEOSPATIAL_POLYLINE_H_

#include <string>

enum PolylineType
{
  POLYLINE_TYPE_POLYPOINT = 0,
  POLYLINE_TYPE_QUAD_BEZIER_CURVE = 1,
  POLYLINE_TYPE_SVGPATH = 2,
  POLYLINE_TYPE_MITSIM    = 3
} ;

namespace sim_mob
{

  class Polyline
  {
  protected:
    
    //Poly-line id
    int id;
    
    //Length of the poly-line
    double length;
    
    //Poly-line type
    int type;
    
    //Scenario
    std::string scenario;
    
  public:
    
    Polyline();
    
    Polyline(const Polyline& src);
    
    virtual ~Polyline();
    
    //Setter for scenario
    void setScenario(std::string scenario);
    
    //Getter for scenario
    std::string getScenario() const;
    
    //Setter for poly-line type
    void setType(int type);
    
    //Getter for poly-line type
    int getType() const;
    
    //Setter for poly-line length
    void setLength(double length);
    
    //getter for poly-line length
    double getLength() const;
    
    //Setter for poly-line id
    void setId(int id);
    
    //Getter for poly-line id
    int getId() const;
  } ;

} /* namespace sim_mob */

#endif /* GEOSPATIAL_POLYLINE_H_ */
