//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <vector>

#include "PolyLine.hpp"
#include "Tag.hpp"
#include "TurningConflict.hpp"

namespace simmobility_network
{

  class TurningPath
  {
  private:
    
    //Unique identifier for the turning path
    unsigned int turningPathId;
    
    //Indicates the id of the lane where the turning path begins
    unsigned int fromLaneId;
    
    //Defines the max speed that the vehicles should adhere to on the turning (m/s)
    double maxSpeed;
    
    //Represents the poly-line for the turning path
    PolyLine* polyLine;
    
    //Holds the additional information
    std::vector<Tag> tags;
    
    //Indicates the id of the lane at which the turning path ends
    unsigned int toLaneId;
    
    //The turning conflicts that lie on the turning. The key for this map is the other turning 
    //path
    std::map<TurningPath *, TurningConflict *> turningConflicts;
    
    //Indicates the id of the turning group to which this turning path belongs
    unsigned int turningGroupId;
    
  public:
    
    TurningPath();
    
    TurningPath(const TurningPath& orig);
    
    virtual ~TurningPath();
    
    //Returns the id of the turning path
    unsigned int getTurningPathId() const;
    
    //Setter for the turning path id
    void setTurningPathId(unsigned int turningPathId);
    
    //Returns the id of the lane from which this turning path originates
    unsigned int getFromLaneId() const;
    
    //Setter for the id of the lane from which this turning path originates
    void setFromLaneId(unsigned int fromLaneId);
    
    //Returns the poly-line for the turning path
    PolyLine* getPolyLine() const;
    
    //Setter for the poly-line of the turning path
    void setPolyLine(PolyLine* polyLine);
    
    //Returns a vector of tags which holds the additional information
    const std::vector<Tag>& getTags() const;
    
    //Setter for the tags field which holds the additional information
    void setTags(std::vector<Tag>& tags);
    
    //Returns the id of the lane where the turning path ends
    unsigned int getToLaneId() const;
    
    //Setter for the id of the lane where the turning path ends
    void setToLaneId(unsigned int toLaneId);
    
    //Returns the id of the turning group of which this turning path is a part
    unsigned int getTurningGroupId() const;
    
    //Setter for the id of the turning group of which this turning path is a part
    void setTurningGroupId(unsigned int turningGroupId);

  } ;
}


