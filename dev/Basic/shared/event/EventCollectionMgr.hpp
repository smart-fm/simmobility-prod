/*
 * EventCollectionManager.h
 *
 *  Created on: Aug 13, 2013
 *      Author: zhang huai peng
 */

#ifndef EVENTCOLLECTIONMANAGER_H_
#define EVENTCOLLECTIONMANAGER_H_

#include "EventPublisher.hpp"
#include "args/EventMessage.hpp"

namespace sim_mob {

namespace event {

class EventCollectionMgr : public EventPublisher {
public:
	EventCollectionMgr();
	virtual ~EventCollectionMgr();

public:
	void SendMessage(MessagePtr message );
	void SendMessage(EventId, MessagePtr message );

private:
	std::vector<MessagePtr> receivingCollector;
	std::vector<MessagePtr> processingCollector;
	friend class EventBusSystem;
};

}

} /* namespace sim_mob */
#endif /* EVENTCOLLECTIONMANAGER_H_ */
