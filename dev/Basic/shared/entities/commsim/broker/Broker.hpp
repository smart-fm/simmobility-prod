//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/thread/condition_variable.hpp>
#include <boost/unordered_map.hpp>

#include "entities/Agent.hpp"
#include "entities/commsim/client/base/ClientRegistration.hpp"
#include "entities/commsim/service/Services.hpp"
#include "entities/commsim/message/Types.hpp"
#include "entities/commsim/message/base/Message.hpp"
#include "entities/commsim/message/base/MessageQueue.hpp"
#include "entities/commsim/message/base/Handler.hpp"
#include "entities/commsim/buffer/BufferContainer.hpp"
#include "entities/commsim/broker/Broker-util.hpp"
#include "util/OneTimeFlag.hpp"
#include "workers/Worker.hpp"

namespace sim_mob {

//Forward Declarations
template <class RET,class MSG>
class MessageFactory;

template<class T>
class Message;

class AgentCommUtilityBase;
class Publisher;
class ConnectionHandler;
class ConnectionServer;
class ClientHandler;
class BrokerBlocker;


 //since we have not created the original key/values, we wont use shared_ptr to avoid crashing
struct MessageElement{
	MessageElement(){}
	MessageElement(boost::shared_ptr<sim_mob::ConnectionHandler> cnnHandler,
			sim_mob::comm::MsgPtr msg):cnnHandler(cnnHandler), msg(msg){}
	boost::shared_ptr<sim_mob::ConnectionHandler> cnnHandler;
	sim_mob::comm::MsgPtr msg;
//typedef boost::tuple<boost::shared_ptr<sim_mob::ConnectionHandler>, sim_mob::msg_ptr > type;
};

struct MessageFactories{
typedef std::map<unsigned int ,boost::shared_ptr<MessageFactory<std::vector<sim_mob::comm::MsgPtr>&, std::string&> > > type;//<client type, roadrunner message factory>
typedef std::map<unsigned int ,boost::shared_ptr<MessageFactory<std::vector<sim_mob::comm::MsgPtr>&, std::string&> > >::iterator iterator;
typedef std::pair<unsigned int ,boost::shared_ptr<MessageFactory<std::vector<sim_mob::comm::MsgPtr>&, std::string&> > > pair;
};

struct PublisherList{
typedef boost::shared_ptr<sim_mob::event::EventPublisher> dataType;
typedef std::map<sim_mob::Services::SIM_MOB_SERVICE, dataType> type;
typedef std::map<sim_mob::Services::SIM_MOB_SERVICE, dataType>::iterator iterator;
typedef std::pair<sim_mob::Services::SIM_MOB_SERVICE, dataType> pair;
};

struct ClientList{
typedef std::map<unsigned int , boost::unordered_map<std::string , boost::shared_ptr<sim_mob::ClientHandler> > > type; //multimap<client type, map<clientID,clienthandler > >
typedef std::map<unsigned int , boost::unordered_map<std::string , boost::shared_ptr<sim_mob::ClientHandler> > >::iterator iterator;
typedef std::pair<unsigned int , boost::unordered_map<std::string , boost::shared_ptr<sim_mob::ClientHandler> > > pair;
typedef std::pair<std::string , boost::shared_ptr<sim_mob::ClientHandler> > IdPair;
};

struct BrokerBlockers{
typedef std::map<unsigned int , boost::shared_ptr<sim_mob::BrokerBlocker> > type; //multimap<Blocker type, BrokerBlocker >   [Blocker type: simmobility agents, android emulator, ns3 simulator  etc]
typedef std::map<unsigned int , boost::shared_ptr<sim_mob::BrokerBlocker> >::iterator iterator;
typedef std::pair<unsigned int , boost::shared_ptr<sim_mob::BrokerBlocker> > IdPair;
};

template<class TYPE>
struct SEND_BUFFER {
typedef boost::unordered_map<boost::shared_ptr<sim_mob::ConnectionHandler>, sim_mob::BufferContainer<TYPE> > type;
typedef typename boost::unordered_map<boost::shared_ptr<sim_mob::ConnectionHandler>, sim_mob::BufferContainer<TYPE> >::iterator iterator;
typedef std::pair<boost::shared_ptr<sim_mob::ConnectionHandler>, sim_mob::BufferContainer<TYPE> > pair;
};

class Broker  : public sim_mob::Agent {
private:
	typedef std::multimap<std::string,ClientRegistrationRequest > ClientWaitList;

	///	Is this Broker currently enabled?
	bool enabled;
	OneTimeFlag configured_;
	//	list of the registered agents and their corresponding communication equipment
//	AgentsMap::type registeredAgents;
	AgentsList REGISTERED_AGENTS;
	///	waiting list for external clients willing to communication with simmobility
	ClientWaitList clientRegistrationWaitingList; //<client type, requestform>
	///	list of authorized clients who have passed the registration process
	ClientList::type clientList; //key note: there can be one agent associated with multiple clients in this list. why? : coz clients of any type are i this list. and any one has associated itself to this agent for its specific type's reason
	///	connection point to outside simmobility
	boost::shared_ptr<sim_mob::ConnectionServer> connection;					//accepts, authenticate and registers client connections
	///	message receive call back function pointer
	boost::function<void(boost::shared_ptr<ConnectionHandler>, std::string)> m_messageReceiveCallback;
	///	list of this broker's publishers
	PublisherList::type publishers;
	///	place to gather outgoing data for each tick
	SEND_BUFFER<Json::Value>::type sendBuffer;
	///	incoming data(from clients to broker) is saved here in the form of messages
	sim_mob::comm::MessageQueue<sim_mob::MessageElement> receiveQueue;
	///	list of classes that process incoming data(w.r.t client type)
	///	to transform the data into messages and assign their message handlers
	MessageFactories::type messageFactories; //<client type, message factory>
	///	list of classes who process client registration requests based on client type
	sim_mob::ClientRegistrationFactory clientRegistrationFactory;
	///	internal controlling container
	std::set<const sim_mob::Agent*> duplicateEntityDoneChecker ;
	///	internal controlling container
	std::set<boost::shared_ptr<sim_mob::ConnectionHandler> > clientDoneChecker;
	///	some control members(//todo: no need to be static as there can be multiple brokers with different requirements)
	static const unsigned int MIN_CLIENTS = 1; //minimum number of registered clients(not waiting list)
	static const unsigned int MIN_AGENTS = 1; //minimum number of registered agents
	///	list of all brokers
	static std::map<std::string, sim_mob::Broker*> externalCommunicators;
	///	used to help deciding whether Broker tick forward or block the simulation
	bool brokerCanTickForward;
	///	container for classes who evaluate wait-for-connection criteria for every type of client
	BrokerBlockers::type clientBlockers; // <client type, BrokerBlocker class>
	///	container for classes who evaluate wait-for-connection criteria for simmobility agents
	BrokerBlockers::type agentBlockers; // <N/A, BrokerBlocker class>
	//various controlling mutexes and condition variables
	boost::mutex mutex_client_request;
	boost::mutex mutex_clientList;
	boost::mutex mutex_clientDone;
	boost::mutex mutex_agentDone;
	boost::condition_variable COND_VAR_CLIENT_REQUEST;
	boost::condition_variable COND_VAR_CLIENT_DONE;
	/*
	 *
	 */
	boost::condition_variable COND_VAR_AGENT_DONE;
	/**
	 * There Can be different types of External Entities(clients)
	 * that are configure to be working with Broker. Each one of
	 * them can have a different criteria to Block the broker until
	 * some condition is met(like client's connection to simmobility).
	 * The following method will examine all those Conditions and reports
	 * back if any client is not connected when it was supposed to be connected.
	 */
	bool isWaitingForAnyClientConnection();
	/**
	 * Returns true if enough agent subscriptions exist to allow the broker
	 * to proceed forward with its update method.
	 */
	bool isWaitingForAgentRegistration() const;
	/**
	 * checks wether an agent(entity) is dead or alive.
	 * Note: this function is not used any more.
	 */
	bool deadEntityCheck(sim_mob::AgentCommUtilityBase * info);
	/**
	 * Returns true if enough clients exist to allow the broker to update.
	 * Note: this function is not used any more.
	 */
	bool clientsQualify() const;
	/**
	 * The Following two methods revise the registration of the registered agents.
	 * Note: these functions are not used any more.
	 */
	void refineSubscriptionList();
	void refineSubscriptionList(sim_mob::Agent * target);
	/**
	 * processes clients requests to be registered with the broker
	 */
	void processClientRegistrationRequests();
	/**
	 * removes a client from the list of registered agents
	 * Note: this function is not used any more.
	 */
	void removeClient(ClientList::iterator it_erase);
	/**
	 * checks to see if a client has sent all what it had to send during the current tick
	 * Note: The client will send an explicit message stating it is 'done' with whatever
	 * it had to send for the current tick.
	 */
	void waitForClientsDone();
	/**
	 * checks to see if every registered agent has completed its operations for the
	 * current tick or not.
	 */
	bool allAgentUpdatesDone();
	/**
	 * internal cleanup at each tick
	 */
	void cleanup();
	/**
	 * 	handlers executed when an agent is going out of simulation(die)
	 */
	virtual void OnEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);
	/**
	 * to be called and identify the agent who has just updated
	 */
	void onAgentUpdate(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const UpdateEventArgs& argums);
	/**
	 * 	is called when a new client is registered with the broker
	 */
	void onClientRegister(sim_mob::event::EventId id/*, sim_mob::event::Context context*/, sim_mob::event::EventPublisher* sender, const ClientRegistrationEventArgs& argums);
	/**
	 * 	publish various data the broker has subscibed to
	 */
	void processPublishers(timeslice now);
	/**
	 * 	sends a signal to clients(through send buffer) telling them broker is ready to receive their data for the current tick
	 */
	void sendReadyToReceive();
	/**
	 * 	sends out data accumulated in the send buffer
	 */
	void processOutgoingData(timeslice now);
	/**
	 * 	handles messages received in the current tick
	 */
	void processIncomingData(timeslice);

public:
	/**
	 * constructor and destructor
	 */
	explicit Broker(const MutexStrategy& mtxStrat, int id=-1);
	/**
	 * 	configure publisher, message handlers and waiting criteria...
	 */
	void configure();
	/**
	 * 	temporary function replacing onAgentUpdate
	 */
	void AgentUpdated(Agent *);
	///	destructor
	~Broker();
	//	enable broker
	void enable();
	/**
	 * 	disable broker
	 */
	void disable();
	/**
	 * 	returns true if broker is enabled
	 */
	bool isEnabled() const;
	/**
	 * 	list of registered agents
	 */
	AgentsList::type &getRegisteredAgents();
	/**
	 * 	list of registered agents + mutex
	 */
	AgentsList::type &getRegisteredAgents(AgentsList::Mutex *mutex);
	/**
	 * 	register an agent
	 */
	bool registerEntity(sim_mob::AgentCommUtilityBase * );
	/**
	 * 	unregister an agent
	 */
	void unRegisterEntity(sim_mob::AgentCommUtilityBase *value);
	/**
	 * 	unregister an agent
	 */
	void unRegisterEntity(sim_mob::Agent * agent);
	/**
	 * 	returns list of clients waiting to be admitted as registered clients
	 */
	ClientWaitList & getClientWaitingList();
	/**
	 * 	returns list of registered clients
	 */
	ClientList::type & getClientList();
	/**
	 *
	 * 	searches for a client of specific ID and Type
	 */
	bool getClientHandler(std::string clientId,std::string clientType, boost::shared_ptr<sim_mob::ClientHandler> &output);
	/**
	 * 	adds to the list of registered clients
	 */
	void insertClientList(std::string ,unsigned int , boost::shared_ptr<sim_mob::ClientHandler>&);
	/**
	 * 	adds a client to the registration waiting list
	 */
	void insertClientWaitingList(std::pair<std::string,ClientRegistrationRequest >);

	///Return an EventPublisher for a given type. Throws an exception if no such type is registered.
	PublisherList::dataType getPublisher(sim_mob::Services::SIM_MOB_SERVICE serviceType);

	/**
	 * 	request to insert into broker's send buffer
	 */
	bool insertSendBuffer(boost::shared_ptr<sim_mob::ConnectionHandler>, const Json::Value&);
	/**
	 * 	callback function executed upon message arrival
	 */
	void messageReceiveCallback(boost::shared_ptr<ConnectionHandler>cnnHadler , std::string message);
	/**
	 * 	The Accessor used by other entities to note the broker's message receive call back function
	 */
	boost::function<void(boost::shared_ptr<ConnectionHandler>, std::string)> getMessageReceiveCallBack();
	/**
	 * 	broker, as an agent, has an update function
	 */
	Entity::UpdateStatus update(timeslice now);
	static std::map<std::string, sim_mob::Broker*> & getExternalCommunicators() ;
	static sim_mob::Broker* getExternalCommunicator(const std::string & value) ;
	/**
	 * 	Add proper entries in simmobility's configuration class
	 */
	static void addExternalCommunicator(const std::string & name, sim_mob::Broker* broker);
	//abstracts & virtuals
	void load(const std::map<std::string, std::string>& configProps);
	bool frame_init(timeslice now);
	Entity::UpdateStatus frame_tick(timeslice now);
	void frame_output(timeslice now);
	bool isNonspatial();
protected:
	/**
	 * 	Wait for clients
	 */
	bool wait();
	/**
	 * 	wait for the registered agents to complete their tick
	 */
	void waitForAgentsUpdates();
	/**
	 * 	wait for a message from the client stating that they are done sending messages for this tick
	 */
	bool isClientDone(boost::shared_ptr<sim_mob::ClientHandler> &);
	/**
	 * 	wait for a message from all of the registered the client stating that they are done sending messages for this tick
	 */
	bool allClientsAreDone();

};


}
