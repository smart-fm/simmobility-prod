//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <list>
#include <map>
#include <set>

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
  
  class IntersectionAccess;

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
    list<IntersectionAccess> receivedRequests;
    
    //Stores the responses sent in the current frame tick
    list<IntersectionAccess> sentResponses;
    
    //Separation time between vehicles following one another (also known as T1)
    double tailgateSeparationTime;

    //Separation time between vehicles with conflicting trajectories (also known as T2)
    double conflictSeparationTime;
    
    //Iterates through the processed requests to find the vehicles that are incompatible with the current request
    void getConflicts(IntersectionAccess request, list<IntersectionAccess> &conflicts);
    
    //Filters out the conflicts which have been allocated access times less than the 
	//access time for current request
    void filterConflicts(double accessTime, list<IntersectionAccess> &conflicts);
    
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
  
  class IntersectionAccess : public Message
  {
  private:
    
    //The arrival time of the person at the intersection
    double arrivalTime;
    
    //The turning that will be used by the person
    int turningId;
    
  public:
    
    IntersectionAccess(const double arrivalTime, int turningId) : 
    arrivalTime(arrivalTime), turningId(turningId)
    {
    }
 
    //Returns the arrival time of the person at the intersection
    double getArrivalTime() const
    {
      return arrivalTime;
    }
    
    int getTurningId() const
    {
      return turningId;
    }

  } ;

  struct CompareArrivalTimes
  {
    bool operator() (IntersectionAccess first, IntersectionAccess second)
    {
      return ( first.getArrivalTime() > second.getArrivalTime() );
    }
  } ;
}
