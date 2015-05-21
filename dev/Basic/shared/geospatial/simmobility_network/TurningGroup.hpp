//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "Tag.hpp"

namespace simmobility_network
{
  enum Rules
  {
    
  };

  class TurningGroup
  {
  private:
    
    //Unique identifier for the turning group
    unsigned int turningGroupId;
    
    //Indicates the link from which this turning group originates
    unsigned int fromLinkId;
    
    //Indicates the node at which this turning group is present
    unsigned int nodeId;
    
    //Indicates the phases of the traffic light during which the vehicles can pass
    std::string phases;
    
    //Stores the turning group rules
    Rules rules;
    
    //Holds the additional information
    Tag *tag;
    
    //Indicates the link at which this turning group terminates
    unsigned int toLinkId;
    
  public:
    
    TurningGroup(unsigned int id, unsigned int fromLink, unsigned int nodeId, std::string phases, Rules rules,
                 Tag *tag, unsigned int toLink);
    
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
    
    //Returns the id of the node at which the turning group is located
    unsigned int getNodeId() const;
    
    //Setter for the node id
    void setNodeId(unsigned int nodeId);
    
    //Returns the signal phases during which vehicles on the turning group can pass
    std::string getPhases() const;
    
    //Setter for the traffic signal phases
    void setPhases(std::string phases);
    
    //Returns the rules for the turning group
    Rules getRules() const;
    
    //Setter for the rules for this turning group
    void setRules(Rules rules);
    
    //Returns a pointer to the tag which holds the additional information
    Tag* getTag() const;
    
    //Setter for the tag field which holds the additional information
    void setTag(Tag* tag);
    
    //Returns the id of the link at which the turning group ends
    unsigned int getToLinkId() const;
    
    //Setter for the id of the link at which the turning group ends
    void setToLinkId(unsigned int toLinkId);
  } ;
}


