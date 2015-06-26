//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <list>
#include <map>
#include <set>
#include <vector>

#include "Agent.hpp"
#include "conf/params/ParameterManager.hpp"
#include "geospatial/MultiNode.hpp"
#include "Person.hpp"

using namespace std;
using namespace sim_mob::messaging;

namespace sim_mob
{
  
  const Message::MessageType MSG_REQUEST_INT_ARR_TIME = 7000000;
  const Message::MessageType MSG_RESPONSE_INT_ARR_TIME = 7000001;
  
  class IntAccessRequest;
  class IntAccessResponse;

  class IntersectionManager : public Agent
  {
  private:
    
    //The id of the intersection manager. The id is the same as the multi-node id
    unsigned int intMgrId;
    
    //The multi-node that the intersection manager manages
    const MultiNode *multinode;
    
    //This map stores the most recent access time granted to a vehicle based on its turning id
    //Key: Turning id, Value: Previous access time
    map<int, double> mapOfPrevAccessTimes;
    
    //Stores the requests to be processed during the upcoming frame tick
    list<const IntAccessRequest *> receivedRequests;
    
    //Separation time between vehicles following one another (also known as T1)
    double tailgateSeparationTime;

    //Separation time between vehicles with conflicting trajectories (also known as T2)
    double conflictSeparationTime;
    
    //Iterates through the processed requests to find the request that are incompatible with the current request
    //and adds them to the vector of incompatible requests
    void getIncompatibleRequests(const IntAccessRequest *request, vector<const IntAccessRequest *> &processedReq,
                                 vector<const IntAccessRequest *> &incompatibleReq);
    
    //Filters out the incompatible requests which have been allocated access times less than the 
	//access time for current request
    void filterOutNonConflictingReq(double accessTime, vector<const IntAccessRequest *> &incompatibleReq);
    
  protected:
    
    //Returns true as the intersection manager is non spatial
    virtual bool isNonspatial();

    //Does nothing - compulsory override from agent class
    virtual void load(const std::map<std::string, std::string>& configProps);
    
    //Called during the first call to update() for a given agent.
    virtual bool frame_init(timeslice now);
    
    //Called during every call to update() for a given agent.
    virtual Entity::UpdateStatus frame_tick(timeslice now);
    
    //Called after frame_tick() for every call to update() for a given agent
    virtual void frame_output(timeslice now);

  public:
    
    //Map holding the pointers to all the intersection manager objects
    static map<unsigned int, IntersectionManager *> intManagers;
    
    IntersectionManager(MutexStrategy const &mutexStrategy, const MultiNode *multinode);
    
    virtual ~IntersectionManager();
    
    //The message handler
    virtual void HandleMessage(Message::MessageType type, const Message& message);
    
    //Returns a pointer to the intersection manager with the given id
    static IntersectionManager* getIntManager(unsigned int id);

  } ;  
  
  class IntAccessRequest : public Message
  {
  private:
    
    //The driver who sent the request
    const Person *person;
    
    //The arrival time of the person (based on the current speed and distance to the intersection)
    double arrivalTime;
    
    //The turning that will be used by the person
    const TurningSection *turning;
    
  public:
    
    IntAccessRequest(const Person *person, const double arrivalTime, const TurningSection *turning) : 
    person(person), arrivalTime(arrivalTime), turning(turning)
    {
    }
    
    //Returns a pointer to the person object
    const Person* getPerson() const
    {
      return person;
    }

    //Returns the earliest arrival time of the person at the intersection
    double getArrivalTime() const
    {
      return arrivalTime;
    }
    
    const TurningSection* getTurning() const
    {
      return turning;
    }

  } ;
  
  class IntAccessResponse : public Message
  {
  private:
    
    //The time at which the vehicle must reach the intersection
    double intAccessTime;
    
  public:
    
    IntAccessResponse(double accessTime) : intAccessTime(accessTime)
    {
    }
  };

  struct CompareArrivalTimes
  {
    bool operator() (const IntAccessRequest *first, const IntAccessRequest *second)
    {
      return ( first->getArrivalTime() > second->getArrivalTime() );
    }
  } ;
}
