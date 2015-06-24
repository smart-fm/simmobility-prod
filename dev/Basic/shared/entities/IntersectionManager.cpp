//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "IntersectionManager.hpp"

using namespace sim_mob;

//Initialise static member
vector<IntersectionManager *> IntersectionManager::intManagers;

IntersectionManager::IntersectionManager(const MutexStrategy& mutexStrategy, const MultiNode *node) :
Agent(mutexStrategy), multinode(node)
{
	Print() << "\nCreating IntMgr " << node->getID();
}

IntersectionManager::~IntersectionManager()
{
}

bool IntersectionManager::frame_init(timeslice now)
{
	return true;
}

Entity::UpdateStatus IntersectionManager::frame_tick(timeslice now)
{
	
	//Intersection managers are not removed
	return UpdateStatus::Continue;
}

void IntersectionManager::frame_output(timeslice now)
{	
}

bool IntersectionManager::isNonspatial()
{
	return true;
}

void IntersectionManager::load(const std::map<std::string,std::string>& configProps)
{
}