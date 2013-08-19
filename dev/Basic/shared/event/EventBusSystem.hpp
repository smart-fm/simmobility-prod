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
	void ProcessTransimition();

protected:

private:
	std::vector<EventCollectionMgr*> childrenManagers;
};

}

} /* namespace sim_mob */
#endif /* EVENTBUSSYSTEM_H_ */
