//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace simmobility_network
{

  class TurningConflict
  {
  private:

    //Conflict id
    unsigned int conflictId;
    
    //Distance of conflict point from start of the first turning
    double firstConflictDistance;
    
    //Id of the first conflicting Turning path
    unsigned int firstTurningId;
      
    //Distance of conflict point from the start of the second turning
    double secondConflictDistance;

    //Id of the second conflicting Turning path
    unsigned int secondTurningId;

    //Holds additional information
    Tag *tag;

  public:
    
    TurningConflict(unsigned int id, double first_cd, unsigned int firstTurningId, double second_cd, unsigned int secondTurningId,
                    Tag *tag);

    TurningConflict(const TurningConflict& tc);

    virtual ~TurningConflict();
    
    //Returns the conflict id
    unsigned int getConflictId() const;
    
    //Sets the conflict id
    void setConflictId(unsigned int conflictId);
    
    //Returns the conflict distance to first turning
    double getFirstConflictDistance() const;
    
    //Sets the conflict distance to the first turning
    void setFirstConflictDistance(double firstConflictDistance);
    
    //Returns the id of the first turning
    unsigned int getFirstTurningId() const;
    
    //Sets the id of the first turning
    void setFirstTurningId(unsigned int firstTurningId);
    
    //Returns the conflict distance to second turning
    double getSecondConflictDistance() const;
    
    //Sets the conflict distance to second turning
    void setSecondConflictDistance(double secondConflictDistance);
    
    //Returns the id of the second turning
    unsigned int getSecondTurningId() const;
    
    //Sets the id of the second turning
    void setSecondTurningId(unsigned int secondTurningId);
    
    //Returns a pointer to the tag which holds the additional information
    Tag* getTag() const;
    
    //Setter for the tag field which holds the additional information
    void setTag(Tag* tag);
    
  } ;
}

