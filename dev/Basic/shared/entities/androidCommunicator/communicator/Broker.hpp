#include "message/Message.hpp"
#include "message/MessageReceiver.hpp"
#include "entities/Agent.hpp"
#include "Server/ASIO_Server.hpp"


#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include "../../communicator/CommunicationSupport.hpp"
namespace sim_mob
{
//class CommunicationSupport;
struct subscription
{
	subscription(sim_mob::CommunicationSupport* cs) :
			 connected(false), agent(&(cs->getEntity())) {
		CommunicationSupport_.reset(cs);
	}

	subscription(
			const sim_mob::Entity * agent_,
			unsigned int clientID_,
			sim_mob::CommunicationSupport * CommunicationSupport_,
			boost::shared_ptr<ConnectionHandler> handler_
			):
				agent(agent_),
				clientID(clientID_),
				CommunicationSupport_(CommunicationSupport_),
				handler(handler_)
	{connected = false;}
	subscription & operator=(const subscription &s)
	{
		agent = s.agent;
		clientID = s.clientID;
		CommunicationSupport_ = s.CommunicationSupport_;
		handler = s.handler;
		connected = s.connected;
	}


	boost::shared_ptr<sim_mob::CommunicationSupport> CommunicationSupport_;
	boost::shared_ptr<ConnectionHandler> handler;
	const sim_mob::Entity * agent;
//	session_ptr session;
	unsigned int clientID;
	bool connected;
};

typedef boost::multi_index_container<
		subscription, boost::multi_index::indexed_by<
		boost::multi_index::random_access<>															//0
    ,boost::multi_index::ordered_unique<boost::multi_index::member<subscription, const sim_mob::Entity * , &subscription::agent> >//1
	,boost::multi_index::ordered_unique<boost::multi_index::member<subscription, unsigned int  , &subscription::clientID> >//2

   >
> subscriptionC;//Link and Crossing Container(multi index)
typedef boost::multi_index::nth_index<subscriptionC, 0>::type subscriberList;
typedef boost::multi_index::nth_index<subscriptionC, 1>::type agentSubscribers;
typedef boost::multi_index::nth_index<subscriptionC, 2>::type clientSubscribers;



typedef subscriberList::reverse_iterator subscriberIterator;
typedef agentSubscribers::iterator agentIterator;
typedef clientSubscribers::iterator clientIterator;


class Broker  : public sim_mob::Agent, public sim_mob::MessageReceiver
{
	enum MessageTypes
	{
		 ANNOUNCE = 1,
		 KEY_REQUEST = 2,
		 KEY_SEND = 3
	};
	static Broker instance;
	//list of agents willing to participate in communication simulation
	//they are catgorized as those who get a connection and those
	//who are waiting to get one.

	subscriptionC subscriptionList;

	//for find, insert, iteration etc
	subscriberList &subscriberList_;
	agentSubscribers &agentSubscribers_;
	clientSubscribers &clientSubscribers_;

	std::map<const sim_mob::Entity*,subscription > agentList;
	std::map<const sim_mob::Entity*,subscription > agentWaitingList;
	//list of available clients ready to be assigned to agents
	std::queue<std::pair<unsigned int,sim_mob::session_ptr > >clientList;

	//accepts, authenticate and registers client connections
	sim_mob::server server_;
	//incoming message handler
	void HandleMessage(MessageType type, MessageReceiver& sender,const Message& message);
	//asio provisions
	boost::asio::io_service io_service_;
	boost::thread io_service_thread; //thread to run the io_service
	void io_service_run(boost::asio::io_service & ); //thread function


public:
	boost::shared_mutex *Communicator_Mutex;
	boost::shared_mutex *Communicator_Mutex_Send;
	boost::shared_mutex *Communicator_Mutex_Receive;
	std::vector<boost::shared_mutex*> mutex_collection;
	subscriptionC &getSubscriptionList();
	explicit Broker(const MutexStrategy& mtxStrat, int id=-1);
	~Broker();
	Entity::UpdateStatus update(timeslice now);
	void load(const std::map<std::string, std::string>& configProps){};
	bool frame_init(timeslice now){};
	Entity::UpdateStatus frame_tick(timeslice now){};
	void frame_output(timeslice now){};
	//assign a client from clientList to an agent in the agentList
//	void assignClient(sim_mob::Entity *agent, std::pair<unsigned int,session_ptr> client);
	std::vector<boost::shared_mutex *> subscribeEntity(sim_mob::CommunicationSupport&);
	static Broker& GetInstance() { return Broker::instance; }
	void turnOnConnections();
	void turnOnConnection(session_ptr);
	bool isNonspatial(){};
};


}
