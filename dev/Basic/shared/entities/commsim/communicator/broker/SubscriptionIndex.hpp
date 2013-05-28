#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>

namespace sim_mob {

//type of services that clients like to subscribed to receive updates from publisher

enum SIM_MOB_SERVICE
{
	SIMMOB_SRV_TIME,
	SIMMOB_SRV_LOCATION
};

std::map<std::string, SIM_MOB_SERVICE> ServiceMap = boost::assign::map_list_of("SIMMOB_SRV_TIME", SIMMOB_SRV_TIME)("SIMMOB_SRV_LOCATION", SIMMOB_SRV_LOCATION);
struct ClientRegistrationRequest
{
	unsigned int clientID;
	unsigned int client_type; //ns3, android emulator, FMOD etc
	std::set<SIM_MOB_SERVICE> requiredServices;
	session_ptr session_;
};
//Forward Declaration
template<class T>
class JCommunicationSupport;

struct registeredClient {
	boost::shared_ptr<ConnectionHandler > cnnHandler;
	sim_mob::JCommunicationSupport* JCommunicationSupport_; //represents a Role, so dont use a boost::share_ptr whose object is created somewhere else. it is dangerous
	const sim_mob::Agent* agent;
	unsigned int clientID;
	unsigned int client_type; //ns3, android emulator, FMOD etc
	std::set<SIM_MOB_SERVICE> requiredServices;
};

struct subscription {
	subscription(sim_mob::JCommunicationSupport* cs)
	: agent(&(cs->getEntity()))
	, clientID(0)
	,handler(boost::shared_ptr<ConnectionHandler>())
	,JCommunicationSupport_(cs), connected(false)
	{}

	const sim_mob::Agent* agent;
	unsigned int clientID;
	int client_type; //ns3, android emulator, FMOD etc
	boost::shared_ptr<ConnectionHandler > handler;
	sim_mob::JCommunicationSupport* JCommunicationSupport_;
	std::set<SIM_MOB_SERVICE> requiredServices;

private:
	bool connected;

public:

	//None of this code appears to do anything:
	///////////////////////////////////////////////////////////////

	//This constructor does not appear to be used. ~Seth
	/*subscription(const sim_mob::Agent* agent_, unsigned int clientID_, sim_mob::JCommunicationSupport* cs,
		boost::shared_ptr<ConnectionHandler > handler_)
	: agent(agent_), clientID(clientID_), JCommunicationSupport_(cs), handler(handler_)
	{
		connected = false;
	}*/

	//This destructor doesn't appear to do anything.
/*	~subscription() {
//		handler.reset();
//		JCommunicationSupport_.reset();
	}*/

	//An assignment operator is not needed; everything here is directly copyable.
	/*subscription& operator=(const subscription& s) {
		agent = s.agent;
		clientID = s.clientID;
		JCommunicationSupport_ = s.JCommunicationSupport_;
		handler = s.handler;
		connected = s.connected;
		return *this;
	}*/

///////////////////////////////////////////////////////////////

};


typedef std::map<const sim_mob::Agent *, JCommunicationSupport* > AgentsMap; //since we have not created the original key/values, we wont use shared_ptr to avoid crashing
typedef std::multimap<unsigned int,ClientRegistrationRequest > ClientWaitList; //<client type,registrationrequestform >
typedef std::multimap<unsigned int , registeredClient> ClientList; //<client type,clientregistrationinfo >

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
