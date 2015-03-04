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

  private:
    
    //Database Id
    int dbId;
    
    //X coordinate of the starting point of the Turning. This is also the end point of the lane 
    //entering the intersection
    double from_xpos;
    
    //Y coordinate of the starting point of the Turning. This is also the end point of the lane 
    //entering the intersection
    double from_ypos;
    
    //X coordinate of the end point of the Turning. This is also the start point of the lane 
    //at which the intersection exits
    double to_xpos;
    
    //Y coordinate of the end point of the Turning. This is also the start point of the lane 
    //at which the intersection exits
    double to_ypos;
    
    //Id of the road section leading into the turning
    std::string from_road_section;
    
    //Id of the road section the turning leads into
    std::string to_road_section;
    
    //Index of the lane leading into the turning
    int from_lane_index;
    
    //Index of the lane the turning leads into
    int to_lane_index;

    //Section id
    std::string sectionId;
    
    //TODO add phases

    //The RoadSegment object leading into the turning
    sim_mob::RoadSegment* fromSeg;
    
    //The RoadSegment object the turning leads into
    sim_mob::RoadSegment* toSeg;

    //Polyline points representing the geometry of the turning
    std::vector<Point2D> polylinePoints;

    //The Lane object leading into the turning
    const sim_mob::Lane* laneFrom;
    
    //The Lane object the turning leads into
    const sim_mob::Lane* laneTo;
    
    //Contains all the turnings in the intersection that have conflicts with this turning
    std::vector<TurningSection *> conflictingTurningSections;
    
    //Contains all the conflicts that this turning has with the other turnings in the intersection
    std::vector<TurningConflict *> turningConflicts;
    
    //The defined max. speed for the turning
    int turningSpeed;

  public:
    TurningSection();
    
    TurningSection(const TurningSection & ts);
    
    //Inserts a turning conflict into the vector turningConflicts
    void addTurningConflict(TurningConflict* turningConflict);
    
    //Returns the vector turningConflicts
    std::vector<TurningConflict*>& getTurningConflicts();
    
    //Inserts a TurningSection into the vector conflictingTurningSections
    void addConflictingTurningSections(TurningSection* conflictingTurningSection);
    
    //Returns the vector conflictingTurningSections
    std::vector<TurningSection*>& getConflictingTurningSections();
    
    //Setter for laneTo
    void setLaneTo(const sim_mob::Lane* laneTo);
    
    //Getter for laneTo
    const sim_mob::Lane* getLaneTo() const;
    
    //Setter for laneFrom
    void setLaneFrom(const sim_mob::Lane* laneFrom);
    
    /// use from,to lane to get start,end polyline points
    void makePolylinePoint();

    //Getter for laneFrom
    const sim_mob::Lane* getLaneFrom() const;
    
    //Setter for polylinePoints
    void setPolylinePoints(std::vector<Point2D> polylinePoints);
    
    //Getter for polylinePoints
    std::vector<Point2D> getPolylinePoints() const;
    
    //Setter for toSeg
    void setToSeg(sim_mob::RoadSegment* toSeg);
    
    //Getter for toSeg
    sim_mob::RoadSegment* getToSeg() const;
    
    //Setter for fromSeg
    void setFromSeg(sim_mob::RoadSegment* fromSeg);
    
    //Getter for fromSeg
    sim_mob::RoadSegment* getFromSeg() const;
    
    //Setter for sectionId
    void setSectionId(std::string sectionId);
    
    //Getter from sectionId
    std::string getSectionId() const;
    
    //Setter for to_lane_index
    void setTo_lane_index(int to_lane_index);
    
    //Getter for to_lane_index
    int getTo_lane_index() const;
    
    //Setter for from_lane_index
    void setFrom_lane_index(int from_lane_index);
    
    //Getter for from_lane_index
    int getFrom_lane_index() const;
    
    //Setter for to_road_section
    void setTo_road_section(std::string to_road_section);
    
    //Getter for to_road_section
    std::string getTo_road_section() const;
    
    //Setter for from_road_section
    void setFrom_road_section(std::string from_road_section);
    
    //Getter for from_road_section
    std::string getFrom_road_section() const;
    
    //Setter for to_ypos
    void setTo_ypos(double to_ypos);
    
    //Getter for to_ypos
    double getTo_ypos() const;
    
    //Setter for to_xpos
    void setTo_xpos(double to_xpos);
    
    //Getter for to_xpos
    double getTo_xpos() const;
    
    //Setter for from_ypos
    void setFrom_ypos(double from_ypos);
    
    //Getter for from_ypos
    double getFrom_ypos() const;
    
    //Setter for from_xpos
    void setFrom_xpos(double from_xpos);
    
    //Getter for from_xpos
    double getFrom_xpos() const;
    
    //Setter for dbId
    void setDbId(int dbId);
    
    //Getter for dbId
    int getDbId() const;
  
    //Returns the TurningConflict between the given turnings (the current one (this) and the parameter)
    TurningConflict* getTurningConflict(const TurningSection* ts);
    
    //Setter for turningSpeed
    void setTurningSpeed(int turningSpeed);
    
    //Getter for turningSpeed
    int getTurningSpeed() const;
  } ;

};// end namespace
