//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include "TurningSection.hpp"

namespace sim_mob
{

class TurningSection;

  class TurningConflict
  {
  private:
    
    //Conflict id
    std::string conflictId;
    
    //Database id
    int dbId;
    
    //DbId of the first conflicting TurningSection
    std::string first_turning;
    
    //DbId of the second conflicting TurningSection
    std::string second_turning;
    
    //Distance of conflict point from start of the first turning
    double first_cd;
    
    //Distance of conflict point from the start of the second turning
    double second_cd;
    
    //The first turning section in the conflict 
    //Note:: First/second doesn't have any significance
    TurningSection *firstTurning;
    
    //The second turning section in the conflict
    //Note:: First/second doesn't have any significance
    TurningSection *secondTurning;

    //Threshold value for accepting/rejecting the gap (and deciding whether to continue/slow down) between
    //conflicting vehicles (in seconds)
    double criticalGap;
    
    //Indicates which turning has a higher priority.
    //0 - equal, 1 - first_turning has higher priority, 2 - second_turning has higher priority
    int priority;

  public:
    TurningConflict();
    
    TurningConflict(const TurningConflict& tc);
    
    //Setter for secondTurning
    void setSecondTurning(TurningSection* secondTurning);
    
    //Getter for secondTurning
    TurningSection* getSecondTurning() const;
    
    //Setter for firstTurning
    void setFirstTurning(TurningSection* firstTurning);
    
    //Getter for firstTurning
    TurningSection* getFirstTurning() const;
    
    //Setter for conflictId
    void setConflictId(std::string conflictId);
    
    //Getter for conflictId
    std::string getConflictId() const;
    
    //Setter for second_cd
    void setSecond_cd(double second_cd);
    
    //Getter for second_cd
    double getSecond_cd() const;
    
    //Setter for first_cd
    void setFirst_cd(double first_cd);
    
    //Getter for first_cd
    double getFirst_cd() const;
    
    //Setter for second_turning
    void setSecond_turning(std::string second_turning);
    
    //Getter for second_turning
    std::string getSecond_turning() const;
    
    //Setter for first_turning
    void setFirst_turning(std::string first_turning);
    
    //Getter for first_turning
    std::string getFirst_turning() const;
    
    //Setter for dbId
    void setDbId(int dbId);
    
    //Getter for dbId
    int getDbId() const;
    
    //Setter for criticalGap
    void setCriticalGap(double criticalGap);
    
    //Getter for criticalGap
    double getCriticalGap() const;
    
    //Setter for priority
    void setPriority(int priority);
    
    //Getter for priority
    int getPriority() const;
  } ;

};// end namespace

