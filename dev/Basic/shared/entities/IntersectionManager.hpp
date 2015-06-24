//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>

#include "Agent.hpp"
#include "geospatial/MultiNode.hpp"

using namespace std;

namespace sim_mob
{

  class IntersectionManager : public Agent
  {
  private:
    
    //The multi-node that the intersection manager manages
    const MultiNode *multinode;
    
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
    
    //Vector holding the pointers to all the intersection manager objects
    static vector<IntersectionManager *> intManagers;
    
    IntersectionManager(MutexStrategy const &mutexStrategy, const MultiNode *multinode);
    
    virtual ~IntersectionManager();

  } ;
}
