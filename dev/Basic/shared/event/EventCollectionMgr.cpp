/*
 * EventCollectionManager.cpp
 *
 *  Created on: Aug 13, 2013
 *      Author: zhang huai peng
 */

#include "EventCollectionMgr.hpp"

namespace sim_mob {

namespace event{

EventCollectionMgr::EventCollectionMgr() {
	// TODO Auto-generated constructor stub

}

EventCollectionMgr::~EventCollectionMgr() {
	// TODO Auto-generated destructor stub
}

void EventCollectionMgr::Update(const timeslice& currTime)
{
	EventManager::Update( currTime );
	ProcessMessages();
}

void EventCollectionMgr::DistributeMessages(std::vector<MessagePtr>& cols)
{
	std::vector<MessagePtr>::iterator it;
	for(it=cols.begin(); it!=cols.end(); it++){
		processingCollector.push_back( (*it) );
	}
}

void EventCollectionMgr::ProcessMessages()
{
	std::vector<MessagePtr>::iterator it;
	for(it=processingCollector.begin(); it!=processingCollector.end(); it++){
		EventId id = (*it)->GetEventId();
		std::vector<unsigned int>& recipients = (*it)->GetRecipients();
		if( recipients.size() == 0)
		{
			Publish( id, *(*it) );
		}
		else
		{
			std::vector<unsigned int>::iterator itRec;
			for(itRec=recipients.begin(); itRec!=recipients.end(); itRec++){
				unsigned int receiverid = (*itRec);
				Publish( id, receiverid, *(*it) );
			}
		}
	}

	processingCollector.clear();
}

void EventCollectionMgr::SendMessage(MessagePtr message )
{
	EventId id = message->GetEventId();
	RegisterEvent( id );
	receivingCollector.push_back( message );
}

}

} /* namespace sim_mob */
