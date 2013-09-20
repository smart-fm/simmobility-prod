//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * EventBusSystem.h
 *
 *  Created on: Aug 13, 2013
 *      Author: zhang huai peng
 */

#ifndef EVENTBUSSYSTEM_H_
#define EVENTBUSSYSTEM_H_

#include "vector"

namespace sim_mob {

namespace event{

class EventCollectionMgr;

class EventBusSystem {
public:
	EventBusSystem();
	virtual ~EventBusSystem();

	void RegisterChildManager(EventCollectionMgr* child);
	void UnregisteChildManager(EventCollectionMgr* child);
	void ProcessTransimission();

public:
	static EventBusSystem* Instance();

protected:

private:
	static EventBusSystem* pInstance;
	std::vector<EventCollectionMgr*> childrenManagers;
};

}

} /* namespace sim_mob */
#endif /* EVENTBUSSYSTEM_H_ */
