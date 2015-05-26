//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>

#include "Tag.hpp"

namespace simmobility_network
{

  class TurningPath;
  
  class TurningConflict
  {
  private:

    //Conflict id
    unsigned int conflictId;
    
    //Threshold value for accepting/rejecting the gap (and deciding whether to continue/slow down) between
    //conflicting vehicles (in seconds)
    double criticalGap;
    
    //Distance of conflict point from start of the first turning
    double firstConflictDistance;
    
    //The first turning path in the conflict 
    //Note:: First/second doesn't have any significance
    TurningPath *firstTurning;
    
    //Id of the first conflicting Turning path
    unsigned int firstTurningId;
    
    //Indicates which turning has a higher priority.
    //0 - equal, 1 - first_turning has higher priority, 2 - second_turning has higher priority
    int priority;
      
    //Distance of conflict point from the start of the second turning
    double secondConflictDistance;
    
    //The second turning section in the conflict 
    //Note:: First/second doesn't have any significance
    TurningPath *secondTurning;

    //Id of the second conflicting Turning path
    unsigned int secondTurningId;

    //Holds additional information
    std::vector<Tag> *tags;

  public:
    
    TurningConflict();

    TurningConflict(const TurningConflict& tc);

    virtual ~TurningConflict();
    
    //Returns the conflict id
    unsigned int getConflictId() const;
    
    //Sets the conflict id
    void setConflictId(unsigned int conflictId);
    
    //Returns the value of the critical gap for the conflict
    double getCriticalGap() const;
    
    //Sets the value of the critical gap for the conflict
    void setCriticalGap(double criticalGap);
    
    //Returns the conflict distance to first turning
    double getFirstConflictDistance() const;    
    
    //Sets the conflict distance to the first turning
    void setFirstConflictDistance(double firstConflictDistance);
    
    //Returns the first turning to which the conflict belongs
    TurningPath* getFirstTurning() const;
    
    //Sets the first turning to which the conflict belongs
    void setFirstTurning(TurningPath* firstTurning);
    
    //Returns the id of the first turning
    unsigned int getFirstTurningId() const;
    
    //Sets the id of the first turning
    void setFirstTurningId(unsigned int firstTurningId);
    
    //Returns the value of the priority for the conflict
    int getPriority() const;
    
    //Sets the value of the priority for the conflict
    void setPriority(int priority);
    
    //Returns the conflict distance to second turning
    double getSecondConflictDistance() const;
    
    //Sets the conflict distance to second turning
    void setSecondConflictDistance(double secondConflictDistance);
    
    //Returns the second turning to which the conflict belongs
    TurningPath* getSecondTurning() const;
    
    //Sets the second turning to which the conflict belongs
    void setSecondTurning(TurningPath* secondTurning);
    
    //Returns the id of the second turning
    unsigned int getSecondTurningId() const;
    
    //Sets the id of the second turning
    void setSecondTurningId(unsigned int secondTurningId);
    
    //Returns a vector of tags which holds the additional information
    const std::vector<Tag>* getTags() const;
    
    //Setter for the tags field which holds the additional information
    void setTags(std::vector<Tag> *tags);
    
  } ;
}

