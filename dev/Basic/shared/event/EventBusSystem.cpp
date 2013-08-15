/*
 * EventBusSystem.cpp
 *
 *  Created on: Aug 13, 2013
 *      Author: zhang huai peng
 */

#include "EventBusSystem.hpp"

namespace sim_mob {

namespace event{

EventBusSystem::EventBusSystem() {
	// TODO Auto-generated constructor stub

}

EventBusSystem::~EventBusSystem() {
	// TODO Auto-generated destructor stub
}

void EventBusSystem::RegisterChildManager(EventCollectionMgr* child)
{
	childrenManagers.push_back(child);
}

void EventBusSystem::UnregisteChildManager(EventCollectionMgr* child)
{
	std::vector<EventCollectionMgr*>::iterator it;
	for(it=childrenManagers.begin(); it!=childrenManagers.end(); it++){
		if( (*it)==child ){
			childrenManagers.erase( it );
			break;
		}
	}
}

void EventBusSystem::ProcessTransimition()
{
	CollectMessages();
	DistributeMessages();
}

void EventBusSystem::CollectMessages()
{

}

void EventBusSystem::DistributeMessages()
{

}

}

} /* namespace sim_mob */
