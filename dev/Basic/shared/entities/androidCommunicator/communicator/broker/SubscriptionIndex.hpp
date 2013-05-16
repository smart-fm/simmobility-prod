#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>

namespace sim_mob
{
class JCommunicationSupport;
struct subscription
{
	///////////////Function Members///////////////////
	subscription(sim_mob::JCommunicationSupport* cs) :
			 connected(false), agent(&(cs->getEntity())) {
		JCommunicationSupport_ = cs;
	}

	subscription(
			const sim_mob::Agent * agent_,
			unsigned int clientID_,
			sim_mob::JCommunicationSupport * JCommunicationSupport_,
			boost::shared_ptr<ConnectionHandler > handler_
			):
				agent(agent_),
				clientID(clientID_),
				JCommunicationSupport_(JCommunicationSupport_),
				handler(handler_)
	{connected = false;}

	subscription & operator=(const subscription &s)
	{
		agent = s.agent;
		clientID = s.clientID;
		JCommunicationSupport_ = s.JCommunicationSupport_;
		handler = s.handler;
		connected = s.connected;
		return *this;
	}
///////////////Data Members///////////////////

	sim_mob::JCommunicationSupport* JCommunicationSupport_;
	boost::shared_ptr<ConnectionHandler > handler;
	const sim_mob::Agent * agent;
//	session_ptr session;
	unsigned int clientID;
	bool connected;
	~subscription()
	{
//		handler.reset();
//		JCommunicationSupport_.reset();
	}

};

typedef boost::multi_index_container<
		subscription, boost::multi_index::indexed_by<
		boost::multi_index::random_access<>															//0
    ,boost::multi_index::ordered_unique<boost::multi_index::member<subscription, const sim_mob::Agent * , &subscription::agent> >//1
	,boost::multi_index::ordered_unique<boost::multi_index::member<subscription, unsigned int  , &subscription::clientID> >//2

   >
> subscriptionC;//Link and Crossing Container(multi index)
typedef boost::multi_index::nth_index<subscriptionC, 0>::type subscriberList;
typedef boost::multi_index::nth_index<subscriptionC, 1>::type agentSubscribers;
typedef boost::multi_index::nth_index<subscriptionC, 2>::type clientSubscribers;



typedef subscriberList::iterator subscriberIterator;
typedef agentSubscribers::iterator agentIterator;
typedef clientSubscribers::iterator clientIterator;
}//namespace sim_mob
