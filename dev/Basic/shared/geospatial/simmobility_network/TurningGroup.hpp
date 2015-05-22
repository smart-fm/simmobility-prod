//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>

#include "Tag.hpp"

namespace simmobility_network
{
  //Defines the rules vehicles must observe at all the turnings in the same group
  enum Rules
  {
    //No stop sign at the turning group
    TURNING_GROUP_RULE_NO_STOP_SIGN = 0,
    
    //Stop sign present at the turning group
    TURNING_GROUP_RULE_STOP_SIGN = 1
  };

  class TurningGroup
  {
  private:
    
    //Unique identifier for the turning group
    unsigned int turningGroupId;
    
    //Indicates the link from which this turning group originates
    unsigned int fromLinkId;
    
    //Indicates the phases of the traffic light during which the vehicles can pass
    std::string phases;
    
    //Stores the turning group rules
    Rules rules;
    
    //Holds the additional information
    std::vector<Tag> tags;
    
    //Indicates the link at which this turning group terminates
    unsigned int toLinkId;
    
    //Defines the visibility of the intersection from the turning group (m/s)
    double visibility;
    
  public:
    
    TurningGroup();
    
    TurningGroup(const TurningGroup& orig);
    
    virtual ~TurningGroup();
    
    //Returns the id of the turning group
    unsigned int getTurningGroupId() const;
    
    //Setter for the id of the turning group
    void setTurningGroupId(unsigned int turningGroupId);
    
    //Returns the id of the link from which this turning group begins
    unsigned int getFromLinkId() const;
    
    //Setter for the id of the link from which this turning group begins
    void setFromLinkId(unsigned int fromLinkId);
    
    //Returns the signal phases during which vehicles on the turning group can pass
    std::string getPhases() const;
    
    //Setter for the traffic signal phases
    void setPhases(std::string phases);
    
    //Returns the rules for the turning group
    Rules getRules() const;
    
    //Setter for the rules for this turning group
    void setRules(Rules rules);
    
    //Returns a vector of tags which holds the additional information
    const std::vector<Tag>& getTags() const;
    
    //Setter for the tags field which holds the additional information
    void setTag(std::vector<Tag>& tags);
    
    //Returns the id of the link at which the turning group ends
    unsigned int getToLinkId() const;
    
    //Setter for the id of the link at which the turning group ends
    void setToLinkId(unsigned int toLinkId);
    
    //Returns the visibility distance of intersection
    double getVisibility() const;

    //Sets the visibility distance of the intersection
    void setVisibility(double visibility);
  } ;
}


