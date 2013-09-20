//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * EventCollectionManager.h
 *
 *  Created on: Aug 13, 2013
 *      Author: zhang huai peng
 */

#ifndef EVENTCOLLECTIONMANAGER_H_
#define EVENTCOLLECTIONMANAGER_H_

#include "EventManager.hpp"
#include "args/EventMessage.hpp"
#include "metrics/Frame.hpp"

namespace sim_mob {

namespace event {

class EventCollectionMgr : public EventManager {
public:
	EventCollectionMgr();
	virtual ~EventCollectionMgr();

public:
	void SendMessage(MessagePtr message );
    void Update(const timeslice& currTime);

protected:
	void ProcessMessages();
	void DistributeMessages(std::vector<MessagePtr>& cols);

private:
	std::vector<MessagePtr> receivingCollector;
	std::vector<MessagePtr> processingCollector;
	friend class EventBusSystem;
};

}

} /* namespace sim_mob */
#endif /* EVENTCOLLECTIONMANAGER_H_ */
