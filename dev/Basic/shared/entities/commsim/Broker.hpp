//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/thread/condition_variable.hpp>
#include <boost/unordered_map.hpp>

#include "conf/ConfigParams.hpp"

#include "entities/Agent.hpp"
#include "entities/commsim/client/ClientType.hpp"
#include "entities/commsim/client/base/ClientRegistration.hpp"
#include "entities/commsim/service/Services.hpp"
#include "entities/commsim/message/Types.hpp"
#include "entities/commsim/message/base/Message.hpp"
#include "entities/commsim/message/base/MessageQueue.hpp"
#include "entities/commsim/message/base/Handler.hpp"
#include "entities/commsim/buffer/BufferContainer.hpp"
#include "entities/commsim/broker/base/Broker-util.hpp"
#include "entities/commsim/broker/base/Common.hpp"

#include "util/OneTimeFlag.hpp"
#include "workers/Worker.hpp"

#include "event/SystemEvents.hpp"
#include "message/MessageBus.hpp"
#include "event/EventPublisher.hpp"

namespace sim_mob {

/**
 * This file contains Broker class as the main component in this file.
 * The rest of the structures are custom data types used in the broker class.
 * Find the documentation at the beginning of each structure/class.
 */

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


//subclassed Eventpublisher coz its destructor is pure virtual
class BrokerPublisher: public sim_mob::event::EventPublisher {
public:
	BrokerPublisher() {
	}
	virtual ~BrokerPublisher() {
	}
};

 //since we have not created the original key/values, we wont use shared_ptr to avoid crashing
struct MessageElement{
	MessageElement(){}
	MessageElement(boost::shared_ptr<sim_mob::ConnectionHandler> cnnHandler, sim_mob::comm::MsgPtr msg) :
		cnnHandler(cnnHandler), msg(msg){}

	boost::shared_ptr<sim_mob::ConnectionHandler> cnnHandler;
	sim_mob::comm::MsgPtr msg;
};


/**
 * A typedef-container for our MessageFactories container type.
 */
struct MessageFactories {
	typedef unsigned int Key;
	typedef boost::shared_ptr<MessageFactory<std::vector<sim_mob::comm::MsgPtr>, std::string> > Value;

	typedef std::map<Key, Value> Type;
	typedef std::pair<Key, Value> Pair;
};


/**
 * A typedef-container for our PublisherList container type.
 */
struct PublisherList {
	typedef sim_mob::Services::SIM_MOB_SERVICE Key;
	typedef boost::shared_ptr<sim_mob::event::EventPublisher> Value;

	typedef std::map<Key, Value> Type;
	typedef std::pair<Key, Value> Pair;
};


/**
 * A typedef-container for our ClientList container type.
 */
struct ClientList {
	typedef sim_mob::comm::ClientType Key;
	typedef boost::unordered_map< std::string , boost::shared_ptr<sim_mob::ClientHandler> > Value;

	typedef std::map<Key, Value> Type;
	typedef std::pair<Key, Value> Pair;
	typedef std::pair<std::string , boost::shared_ptr<sim_mob::ClientHandler> > ValuePair;

};


/**
 * A typedef-container for our BrokerBlockers container type.
 * [Blocker type: simmobility agents, android emulator, ns3 simulator  etc]
 */
struct BrokerBlockers {
	typedef unsigned int Key;
	typedef boost::shared_ptr<sim_mob::BrokerBlocker> Value;

	typedef std::map<Key, Value> Type;
	typedef std::pair<Key, Value> Pair;
};

/**
 * Helper struct: items in our SendBuffer
 */
struct SendBufferItem {
	boost::shared_ptr<sim_mob::ClientHandler> client; //The client sending this message.
	sim_mob::comm::MsgData msg; //The message being sent.
	SendBufferItem() {}
	SendBufferItem(boost::shared_ptr<sim_mob::ClientHandler> client, sim_mob::comm::MsgData msg) : client(client), msg(msg) {}
};

/**
 * A typedef-container for our SendBuffer container type.
 * NOTE: This was only used here, so I am removing the template parameter.
 */
struct SendBuffer {
	//The base types stored in our container.
	typedef boost::shared_ptr<sim_mob::ConnectionHandler> Key;
	typedef sim_mob::BufferContainer<SendBufferItem> Value;

	//The container itself and a "pair" type.
	typedef boost::unordered_map<Key, Value> Type;
	typedef std::pair<Key, Value> Pair;
};

/**
 * This class is the heart of communication simulator
 * It is responsible for configuring and managing communication simulator.
 * the update method of broker performs the following:
 * - managing the synchronization(with internal and external entities)
 * - Processing the incoming requests for registration(internal and external)
 * - Processing the incoming/outgoing messages
 * - clean up
 * - other application specific settings
 * It is advisable to subclass from broker to make customized configuration/implementation
 */
class Broker  : public sim_mob::Agent {
protected:
	struct ClientWaiting {
		ClientRegistrationRequest request;
		bool uniqueSocket;
		ClientWaiting(ClientRegistrationRequest request, bool uniqueSocket) : request(request), uniqueSocket(uniqueSocket) {}
	};

	//clientType => ClientWaiting
	typedef std::multimap<std::string, ClientWaiting> ClientWaitList;

	///the external communication entity that is using this broker as interface to from simmobility
	std::string commElement;//"roadrunner", "stk",...etc
	std::string commMode; //android-only, android-ns3,...etc

	///	Is this Broker currently enabled?
	bool enabled;
	OneTimeFlag configured_;
	//	list of the registered agents and their corresponding communication equipment
//	AgentsMap::type registeredAgents;
	AgentsList REGISTERED_AGENTS;
	///	waiting list for external clients willing to communication with simmobility
	ClientWaitList clientRegistrationWaitingList; //<client type, requestform>
	///	list of authorized clients who have passed the registration process
	ClientList::Type clientList; //key note: there can be one agent associated with multiple clients in this list. why? : coz clients of any type are i this list. and any one has associated itself to this agent for its specific type's reason

	std::vector<session_ptr> waitingWHOAMI_List;

	///	connection point to outside simmobility
	boost::shared_ptr<sim_mob::ConnectionServer> connection;					//accepts, authenticate and registers client connections
	///	message receive call back function pointer
	boost::function<void(boost::shared_ptr<ConnectionHandler>, std::string)> m_messageReceiveCallback;
	///	list of this broker's publishers
	PublisherList::Type publishers;

	///Broker's Publisher
	BrokerPublisher publisher;
	///services to be published by the broker's publisher
	std::vector<sim_mob::Services::SIM_MOB_SERVICE> serviceList;

	///	place to gather outgoing data for each tick
	SendBuffer::Type sendBuffer;

	///	incoming data(from clients to broker) is saved here in the form of messages
	sim_mob::comm::MessageQueue<sim_mob::MessageElement> receiveQueue;
	///	list of classes that process incoming data(w.r.t client type)
	///	to transform the data into messages and assign their message handlers
	MessageFactories::Type messageFactories; //<client type, message factory>
	///	list of classes who process client registration requests based on client type
//	sim_mob::ClientRegistrationFactory clientRegistrationFactory;
	//todo this will be configured in the configure() method and
	//replace the above "clientRegistrationFactory" member for simplicity
	std::map<comm::ClientType, boost::shared_ptr<sim_mob::ClientRegistrationHandler> > ClientRegistrationHandlerMap;
	//	publishes an event when a client is registered with the broker
	sim_mob::ClientRegistrationPublisher registrationPublisher;
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
	BrokerBlockers::Type clientBlockers; // <client type, BrokerBlocker class>

	///	container for classes who evaluate wait-for-connection criteria for simmobility agents
	BrokerBlockers::Type agentBlockers; // <N/A, BrokerBlocker class>

	//various controlling mutexes and condition variables
	boost::mutex mutex_client_request;
	boost::mutex mutex_clientList;
	boost::mutex mutex_clientDone;
	boost::mutex mutex_agentDone;
	boost::condition_variable COND_VAR_CLIENT_REQUEST;
	boost::condition_variable COND_VAR_CLIENT_DONE;

	//Mutex for the waitingWHOAMI list.
	boost::mutex mutex_WaitingWHOAMI;

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
	virtual void processClientRegistrationRequests();
	/**
	 * removes a client from the list of registered agents
	 * Note: this function is not used any more.
	 */
	void removeClient(ClientList::Type::iterator it_erase);
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
	 * The number and order of arguments are as per EventPublisher guidelines.
	 * To avoid comment duplication, refer to the wiki and the corresponding source code for more information
	 */
	virtual void onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);
	/**
	 * to be called and identify the agent who has just updated
	 * The number and order of arguments are as per EventPublisher guidelines.
	 * To avoid comment duplication, refer to the wiki and the corresponding source code for more information
	 */
	virtual void onAgentUpdate(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const UpdateEventArgs& argums);
	/**
	 * 	actuall implementation of onAgentUpdate
	 */
	void agentUpdated(const Agent* target );
	/**
	 * 	is called when a new client is registered with the broker
	 */
	virtual void onClientRegister(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const ClientRegistrationEventArgs& argums);
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
	explicit Broker(const MutexStrategy& mtxStrat, int id=-1, std::string commElement_ = "", std::string commMode_ = "");
	/**
	 * 	configure publisher, message handlers and waiting criteria...
	 */
	virtual void configure();
	/**
	 * 	temporary function replacing onAgentUpdate
	 */
	void AgentUpdated(Agent *);
	~Broker();


	///Retrieve the connection server (used to shoehorn in new clients.
	///NOTE: This is VERY RISKY; it might be a better idea to intercept "NEW_CLIENT" messages
	///      in Broker and have the Broker force the new connection, rather than going through
	///      a NewClientHandler.
	boost::shared_ptr<sim_mob::ConnectionServer> getConnectionServer();


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
	//ClientWaitList & getClientWaitingList();

	///Returns the size of the list of clients waiting to be admitted as registered clients.
	size_t getClientWaitingListSize() const;

	/**
	 * 	returns list of registered clients
	 */
	ClientList::Type & getClientList();
	/**
	 *
	 * 	searches for a client of specific ID and Type
	 */
	bool getClientHandler(std::string clientId,std::string clientType, boost::shared_ptr<sim_mob::ClientHandler> &output);

	/**
	 * 	adds to the list of registered clients
	 */
	void insertClientList(std::string, comm::ClientType , boost::shared_ptr<sim_mob::ClientHandler>&);

	/**
	 * 	adds a client to the registration waiting list
	 */
	void insertClientWaitingList(std::string clientType, ClientRegistrationRequest request, bool uniqueSocket);


	/**
	 * When a new connection is registered (or a NEW_CLIENT message goes out), the
	 *   ConnectionServer sends out a WHOAREYOU message, and then waits for a response.
	 *   Since this response comes in asynchronously, the session pointer is
	 *   pushed to the Broker using this function. It is then paired up with an Agent
	 *   upon receiving a WHOAMI message through the normal channels.
	 * NOTE: For now, this pairing is arbitrary in the case of multiple connections.
	 *       To ensure that ONLY the agent who received the WHOAREYOU can respond, one
	 *       might add a unique token to the WHOAREYOU message that is then relayed back
	 *       in the WHOAMI (but at the moment this is not necessary.
	 */
	void insertIntoWaitingOnWHOAMI(session_ptr session);


	///Return an EventPublisher for a given type. Throws an exception if no such type is registered.
//	PublisherList::Value getPublisher(sim_mob::Services::SIM_MOB_SERVICE serviceType);
	sim_mob::event::EventPublisher & getPublisher();
	/**
	 * Accessor to client registration publisher
	 */
	sim_mob::ClientRegistrationPublisher &getRegistrationPublisher();

	/**
	 * 	request to insert into broker's send buffer
	 */
	bool insertSendBuffer(boost::shared_ptr<sim_mob::ClientHandler> client, const Json::Value& msg);

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
