//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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

EventBusSystem* EventBusSystem::pInstance = 0;

EventBusSystem* EventBusSystem::Instance()
{
	if(pInstance == 0 ) {
		pInstance = new EventBusSystem();
	}

	return pInstance;
}

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

void EventBusSystem::ProcessTransimission()
{
	std::vector<EventCollectionMgr*>::iterator it;
	std::vector<MessagePtr> collectors;
	for(it=childrenManagers.begin(); it!=childrenManagers.end(); it++){
		std::vector<MessagePtr>& col = (*it)->receivingCollector;
		collectors.insert(collectors.begin(), col.begin(), col.end());
		col.clear();
	}

	/*for(std::vector<MessagePtr>::iterator it=collectors.begin(); it!=collectors.end(); it++){
		std::cout << "message id is " << (*it)->GetEventId() << std::endl;
	}*/

	for(it=childrenManagers.begin(); it!=childrenManagers.end(); it++){
		(*it)->DistributeMessages( collectors );
	}
}

}

} /* namespace sim_mob */
