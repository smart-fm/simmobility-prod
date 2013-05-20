#pragma once
//#include "message/Message.hpp"
//#include "message/MessageReceiver.hpp"
#include "entities/commsim/communicator/message/base/MessageFactory.hpp"
#include "entities/commsim/communicator/message/base/MessageQueue.hpp"
#include "entities/Agent.hpp"
//broker specific
#include "SubscriptionIndex.hpp"
#include "entities/commsim/communicator/server/Server.hpp"
#include "entities/commsim/comm_support/JCommunicationSupport.hpp"
#include "entities/commsim/communicator/service/services.hpp"
#include "entities/commsim/communicator/buffer/BufferContainer.hpp"

//external libraries
#include <boost/thread/condition_variable.hpp>
namespace sim_mob
{

class Broker  : public sim_mob::Agent//, public sim_mob::MessageReceiver
{
public:
	explicit Broker(const MutexStrategy& mtxStrat, int id=-1);
	~Broker();

	//static Broker& GetInstance() { return Broker::instance; }

private:
	AgentsMap registeredAgents;
	ClientWaitList clientRegistrationWaitingList; //<client type, requestform>
	ClientList clientList;
	boost::shared_ptr<sim_mob::ConnectionServer> connection;					//accepts, authenticate and registers client connections

	static const unsigned int MIN_CLIENTS = 1; //minimum number of subscribed clients

	typedef void (JCommunicationSupport::*setConnected)(void);

	//impl-1
	enum MessageTypes {
		 ANNOUNCE = 1,
		 KEY_REQUEST = 2,
		 KEY_SEND = 3
	};

	std::map<std::string, MessageTypes> MessageMap;

	//Broker's main buffers
	std::map<const sim_mob::Agent *, sim_mob::BufferContainer > sendBufferMap; //temporarily used, later the buffer of the agent's communicationsupport will be used
	sim_mob::BufferContainer sendBuffer;//apparently useless for this demo
	sim_mob::BufferContainer receiveBuffer;

	//impl-2
	sim_mob::comm::MessageQueue receiveQueue;
	boost::shared_ptr<MessageFactory<msg_ptr, std::string> > messageFactory;


	//static Broker instance;
	//list of agents willing to participate in communication simulation
	//they are catgorized as those who get a connection and those
	//who are waiting to get one.

	subscriptionC subscriptionList;

	//for find, insert, iteration etc
	subscriberList &subscriberList_;
	agentSubscribers &agentSubscribers_;
	clientSubscribers &clientSubscribers_;

	//a set of publishers who provide services to subscribers in each tick
	std::vector<sim_mob::Publisher*> publishers;

	std::set<const sim_mob::Agent*> duplicateEntityDoneChecker ;
	sim_mob::ClientRegistrationFactory clientRegistrationFactory;
	std::map<const sim_mob::Agent*,subscription > agentList;
	std::map<const sim_mob::Agent*,subscription > agentWaitingList;
	//list of available clients ready to be assigned to agents
	std::queue<boost::tuple<unsigned int,unsigned int,sim_mob::session_ptr > >clientList; //   <client_ID, client_type, client_active_session>


	//incoming message handler
	//asio provisions
//	boost::shared_ptr<boost::asio::io_service> io_service_;
//	boost::thread io_service_thread; //thread to run the io_service
	void io_service_run(boost::shared_ptr<boost::asio::io_service> ); //thread function
	void clientEntityAssociation(subscription subscription_, const boost::tuple<unsigned int,unsigned int,sim_mob::session_ptr >& availableClient);
	bool deadEntityCheck(sim_mob::JCommunicationSupport & info);
	void refineSubscriptionList();
//	void HandleMessage(MessageType type, MessageReceiver& sender,const Message& message);

	///Returns true if enough subscriptions exist to allow the broker to update.
	bool subscriptionsQualify() const;

	///Returns true if enough clients exist to allow the broker to update.
	bool clientsQualify() const;



public:
	boost::shared_ptr<boost::mutex> Broker_Client_Mutex;
	boost::shared_ptr<boost::shared_mutex> Broker_Mutex;
	boost::shared_ptr<boost::shared_mutex> Broker_Mutex_Send;
	boost::shared_ptr<boost::shared_mutex> Broker_Mutex_Receive;
	std::vector<boost::shared_ptr<boost::shared_mutex > > mutex_collection;
	boost::shared_ptr<boost::condition_variable> client_register;

	AgentsMap & getRegisteredAgents();
	ClientWaitList & getClientWaitingList();
	ClientList & getClientList();
	void processClientRegistrationRequests();
	sim_mob::BufferContainer &getSendBuffer();
	sim_mob::BufferContainer &getReceiveBuffer();
	void insertSendBuffer(DataElement&);
//	void insertSendBuffer(std::vector<DataElement> &);
//	void insertSendBuffer(sim_mob::BufferContainer&);

	Entity::UpdateStatus update(timeslice now);
	bool allAgentUpdatesDone();

	inline boost::shared_ptr<boost::mutex> getBrokerMutex();
	inline boost::shared_ptr<boost::shared_mutex> getBrokerMutexSend();
	inline boost::shared_ptr<boost::shared_mutex> getBrokerMutexReceive();
	//set to true when there are enough number of subscribers
	//this is used by the Broker to
	//qualifies itself to either
	//-process in/out messages
	//-block the update function and wait for enough number of agents&clients to register
	//-return from update() in order not to block &disturb the simulation
	bool brokerInOperation;



//	bool handleANNOUNCE(std::string);
//	bool handleKEY_REQUEST(std::string data);
//	bool handleKEY_SEND(std::string data);
//	void handleReceiveMessage(std::string);

	void processOutgoingData(timeslice now);
	void preparePerTickData(timeslice now);
	void processIncomingData(timeslice);

	void unicast(const sim_mob::Agent *, std::string);//not used now

	//abstract vitual
	void load(const std::map<std::string, std::string>& configProps){};
	bool frame_init(timeslice now);
	Entity::UpdateStatus frame_tick(timeslice now);
	void frame_output(timeslice now){};
	bool isNonspatial(){ return true; };

	void enable();
	void disable();
	bool isEnabled() const;

	//assign a client from clientList to an agent in the agentList
//	void assignClient(sim_mob::Entity *agent, std::pair<unsigned int,session_ptr> client);

	void processClientRegistrationRequests();
	bool processEntityWaitingList();
	void addAgentToWaitingList(sim_mob::JCommunicationSupport & value, subscription &subscription_);
	bool subscribeEntity(sim_mob::JCommunicationSupport * );
	bool unSubscribeEntity(sim_mob::JCommunicationSupport &value);
	bool unSubscribeEntity(const sim_mob::Agent * agent);
	subscriptionC &getSubscriptionList();

protected:
	///Wait for clients; return "false" to jump out of the loop.
	bool waitForClients();

	//Is this Broker currently enabled?
	bool enabled;


};


}
