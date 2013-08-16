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
		Publish( id, *(*it) );
	}

	processingCollector.clear();
}

void EventCollectionMgr::SendMessage(MessagePtr message )
{
	EventId id = message->GetEventId();
	SendMessage(id, message);
}

void EventCollectionMgr::SendMessage(EventId id, MessagePtr message )
{
	RegisterEvent( id );
	receivingCollector.push_back( message );
}


}

} /* namespace sim_mob */
