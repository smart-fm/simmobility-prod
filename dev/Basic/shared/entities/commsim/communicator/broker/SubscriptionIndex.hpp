//obsolete
#pragma once
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/assign/list_of.hpp> // for 'map_list_of()'

//#include "entities/commsim/communicator/connection/Connection.hpp"

namespace sim_mob {

//type of services that clients like to subscribed to receive updates from publisher


//struct subscription {
//	subscription(sim_mob::JCommunicationSupport* cs)
//	: agent(&(cs->getEntity()))
//	, clientID(0)
//	,handler(boost::shared_ptr<ConnectionHandler>())
//	,JCommunicationSupport_(cs), connected(false)
//	{}
//
//	const sim_mob::Agent* agent;
//	unsigned int clientID;
//	int client_type; //ns3, android emulator, FMOD etc
//	boost::shared_ptr<ConnectionHandler > handler;
//	sim_mob::JCommunicationSupport* JCommunicationSupport_;
//	std::set<SIM_MOB_SERVICE> requiredServices;
//
//private:
//	bool connected;
//
//public:
//
//
//};



}//namespace sim_mob












//
//typedef boost::multi_index_container<
//		subscription, boost::multi_index::indexed_by<
//		boost::multi_index::random_access<>															//0
//    ,boost::multi_index::ordered_unique<boost::multi_index::member<subscription, const sim_mob::Agent * , &subscription::agent> >//1
//	,boost::multi_index::ordered_unique<boost::multi_index::member<subscription, unsigned int  , &subscription::clientID> >//2
//
//   >
//> subscriptionC;//Link and Crossing Container(multi index)
//typedef boost::multi_index::nth_index<subscriptionC, 0>::type subscriberList;
//typedef boost::multi_index::nth_index<subscriptionC, 1>::type agentSubscribers;
//typedef boost::multi_index::nth_index<subscriptionC, 2>::type clientSubscribers;
//
//
//
//typedef subscriberList::iterator subscriberIterator;
//typedef agentSubscribers::iterator agentIterator;
//typedef clientSubscribers::iterator clientIterator;

