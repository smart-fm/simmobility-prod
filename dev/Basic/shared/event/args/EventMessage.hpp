/*
 * EventMessage.h
 *
 *  Created on: Aug 13, 2013
 *      Author: zhang huai peng
 */

#ifndef EVENTMESSAGE_H_
#define EVENTMESSAGE_H_

#include "EventArgs.hpp"
#include <boost/thread.hpp>

namespace sim_mob {

namespace event{

class EventMessage : public EventArgs {
public:
	EventMessage();
	virtual ~EventMessage();

public:
	void SetEventId(EventId id);
	void SetDuration(unsigned int duration);
	void SetDetention(unsigned int detention);
	void SetRecipients(unsigned int receivers[], int length);
	void SetRecipients(std::vector<unsigned int> receivers){ recipients=receivers; }
	EventId GetEventId() { return eventId; }
	std::vector<unsigned int>& GetRecipients() { return recipients; }

private:
	EventId eventId;
	unsigned int duration;
	unsigned int detention;
	std::vector<unsigned int> recipients;
	void* data;
};

typedef boost::shared_ptr<EventMessage> MessagePtr;

}

} /* namespace sim_mob */
#endif /* EVENTMESSAGE_H_ */
