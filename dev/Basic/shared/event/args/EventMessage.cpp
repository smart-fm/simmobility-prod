/*
 * EventMessage.cpp
 *
 *  Created on: Aug 13, 2013
 *      Author: zhang huai peng
 */

#include "EventMessage.hpp"

namespace sim_mob {

namespace event {

EventMessage::EventMessage() : eventId(0), duration(0), detention(0), data(0){
	// TODO Auto-generated constructor stub
}

EventMessage::~EventMessage() {
	// TODO Auto-generated destructor stub
}

void EventMessage::SetEventId(EventId id) {
	eventId = id;
}

void EventMessage::SetRecipients(unsigned int receivers[], int length)
{
	std::vector<unsigned int> recs(receivers, receivers + length );
	recipients = recs;
}


}

} /* namespace sim_mob */
