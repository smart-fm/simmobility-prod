/*
 * EventBusSystem.cpp
 *
 *  Created on: Aug 13, 2013
 *      Author: zhang huai peng
 */

#include "EventBusSystem.hpp"
#include "EventCollectionMgr.hpp"

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
	std::vector<EventCollectionMgr*>::iterator it;
	std::vector<MessagePtr> collectors;
	for(it=childrenManagers.begin(); it!=childrenManagers.end(); it++){
		std::vector<MessagePtr>& col = (*it)->receivingCollector;
		collectors.insert(collectors.begin(), col.begin(), col.end());
	}

	for(it=childrenManagers.begin(); it!=childrenManagers.end(); it++){
		(*it)->DistributeMessages( collectors );
	}
}

void EventBusSystem::CollectMessages()
{

}

void EventBusSystem::DistributeMessages()
{

}

}

} /* namespace sim_mob */
