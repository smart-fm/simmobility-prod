//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <list>
#include <queue>

#include <boost/thread/condition_variable.hpp>
#include <boost/unordered_map.hpp>

#include "conf/ConfigParams.hpp"

#include "entities/Agent.hpp"
#include "entities/commsim/client/ClientType.hpp"
#include "entities/commsim/client/ClientRegistration.hpp"
#include "entities/commsim/service/Services.hpp"
#include "entities/commsim/broker/Broker-util.hpp"
#include "entities/commsim/broker/Common.hpp"
#include "entities/commsim/message/Handlers.hpp"
#include "entities/commsim/message/ThreadSafeQueue.hpp"
#include "entities/commsim/serialization/CommsimSerializer.hpp"
#include "entities/commsim/wait/WaitForAgentRegistration.hpp"
#include "entities/commsim/wait/WaitForAndroidConnection.hpp"
#include "entities/commsim/wait/WaitForNS3Connection.hpp"
#include "entities/commsim/connection/ConnectionServer.hpp"

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


namespace {
//TEMPORARY: I just need an easy way to disable output for now. This is *not* the ideal solution.
const bool EnableDebugOutput = false;
} //End unnamed namespace



/**
 * A typedef-container for our ClientList container type.
 */
struct ClientList {
	typedef std::map< std::string , boost::shared_ptr<sim_mob::ClientHandler> > Type;
};




///Isolates external Broker functionality.
class BrokerBase {
public:
	virtual ~BrokerBase() {}

	//Used by: TODO
	virtual void onMessageReceived(boost::shared_ptr<ConnectionHandler>cnnHadler, const BundleHeader& header, std::string message) = 0;

	//Used by the WhoAreYouProtocol when clients connect.
	virtual void insertIntoWaitingOnWHOAMI(const std::string& token, boost::shared_ptr<sim_mob::ConnectionHandler> newConn) = 0;

	//Used by ClientRegistration when a ClientHandler object has been created. Failing to save the ClientHandler here will lead to its destruction.
	virtual void insertClientList(std::string, comm::ClientType , boost::shared_ptr<sim_mob::ClientHandler>&) = 0;
	virtual AgentsList::type& getRegisteredAgents(AgentsList::Mutex* mutex) = 0;
	virtual sim_mob::event::EventPublisher& getPublisher() = 0;

	//Used by the BrokerBlocker subclasses. Hopefully we can further abstract these.
	virtual size_t getRegisteredAgentsSize() const = 0;
	virtual const ClientList::Type& getAndroidClientList() const = 0;

	//Used by the Handlers to react to different messages.
	virtual bool insertSendBuffer(boost::shared_ptr<sim_mob::ClientHandler> client, const std::string& message) = 0;
	virtual boost::shared_ptr<sim_mob::ClientHandler> getAndroidClientHandler(std::string clientId) const = 0;
	virtual boost::shared_ptr<sim_mob::ClientHandler> getNs3ClientHandler() const = 0;
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
class Broker : public sim_mob::Agent, public sim_mob::BrokerBase {
public:
	explicit Broker(const MutexStrategy& mtxStrat, int id=-1, std::string commElement_ = "", std::string commMode_ = "");
	virtual ~Broker();

protected:
	 //since we have not created the original key/values, we wont use shared_ptr to avoid crashing
	struct MessageElement{
		MessageElement(){}
		MessageElement(boost::shared_ptr<sim_mob::ConnectionHandler> cnnHandler, const MessageConglomerate& conglom) : cnnHandler(cnnHandler), conglom(conglom){}

		boost::shared_ptr<sim_mob::ConnectionHandler> cnnHandler;
		MessageConglomerate conglom;
	};

	///Helper typedef: key for our SendBuffer
	struct SendBuffer {
		typedef boost::shared_ptr<sim_mob::ClientHandler> Key;
	};

	///BrokerPublisher class. No documentation provided.
	class BrokerPublisher : public sim_mob::event::EventPublisher {
	public:
		virtual ~BrokerPublisher() {}
	};

	///ClientRegistrationPublisher class. No documentation provided.
	class ClientRegistrationPublisher : public sim_mob::event::EventPublisher {
	public:
		virtual ~ClientRegistrationPublisher() {}
	};

	struct ClientWaiting {
		ClientRegistrationRequest request;
		boost::shared_ptr<sim_mob::ConnectionHandler> existingConn;
		ClientWaiting(ClientRegistrationRequest request=ClientRegistrationRequest(), boost::shared_ptr<sim_mob::ConnectionHandler> existingConn=boost::shared_ptr<sim_mob::ConnectionHandler>()) :
			request(request), existingConn(existingConn) {}
	};

	///Helper struct for checking if a Connection has received updates from all its Clients.
	struct ConnClientStatus {
		int total;
		int done;
		ConnClientStatus() : total(0), done(0) {}
	};

private:
	///List of all known tokens and their associated ConnectionHandlers.
	///This list is built over time, as new connections/Agents are added (ConnectionHandlers should never be removed).
	/// THREADING: This data structure is modified by parallel threads.
	std::map<std::string, boost::shared_ptr<sim_mob::ConnectionHandler> > tokenConnectionLookup;
	boost::mutex mutex_token_lookup; ///<Mutex to lock tokenConnectionLookup.

	///Waiting list for external Android/Ns-3 clients willing to communication with Sim Mobility.
	std::queue<ClientWaiting> clientWaitListAndroid;
	std::queue<ClientWaiting> clientWaitListNs3;
	boost::mutex mutex_client_wait_list; ///<Mutex for locking the clientWaitListX variables.


protected:
	///Lookup for message handlers by type.
	HandlerLookup handleLookup;

	///the external communication entity that is using this broker as interface to from simmobility
	std::string commElement; //"roadrunner", "stk",...etc
	std::string commMode;    //android-only, android-ns3,...etc


	///List of (Sim Mobility) Agents that have registered themselves with the Broker.
	AgentsList registeredAgents;

	///List of Android clients that have completed registration with the Broker.
	ClientList::Type registeredAndroidClients;

	///List of NS-3 clients that have completed registration with the Broker. (Note: There should only be zero or one).
	ClientList::Type registeredNs3Clients;


	///Manages all incoming/outgoing TCP connections. Creates a new ClientHandler for each one, and may multiplex ConnectionHandlers.
	ConnectionServer connection;

	///Broker's Publisher
	BrokerPublisher publisher;

	//Publishes an event when a client is registered with the broker
	ClientRegistrationPublisher registrationPublisher;


	///Place to gather outgoing data for each tick, on a per-ClientHandler basis (that's how OngoingSerialization works).
	std::map<SendBuffer::Key, OngoingSerialization> sendBuffer;

	//Lock the send buffer (note: I am not entirely sure how thread-safe the sendBuffer is, so I am locking it just in case).
	//TODO: In the future, all clients on the same Broker should share a non-locked list of OngoingSerializations,
	//      and the Broker can just collate these.
	boost::mutex mutex_send_buffer;

	///Incoming data (from clients to broker) is saved here in the form of messages
	sim_mob::ThreadSafeQueue<MessageElement> receiveQueue;

	///Helper class used to handle client registration.
	ClientRegistrationHandler registrationHandler;

	///This map tracks the number of ClientHandlers connected per ConnectionHandler, as well as the number (per tick)
	///  that have already sent the CLIENT_MESSAGES_DONE message. It is locked by its associated mutex.
	std::map<boost::shared_ptr<sim_mob::ConnectionHandler>, ConnClientStatus> clientDoneChecklist;
	boost::mutex mutex_client_done_chk;

	//Broker singleton.
	//TODO: This is not really a singleton; we set/get it in various places. But we need a way of communicating the Broker to the
	//      Agents. For now, this is fine --we only need to clean this up once/if we have multiple Brokers in the system at once.
	static Broker* single_broker;

	WaitForAndroidConnection waitAndroidBlocker;
	WaitForNS3Connection waitNs3Blocker;

	///	container for classes who evaluate wait-for-connection criteria for simmobility agents
	WaitForAgentRegistration waitAgentBlocker;

	//various controlling mutexes and condition variables
	boost::mutex mutex_clientList;
	boost::mutex mutex_clientDone;
	boost::mutex mutex_agentDone;
	boost::condition_variable COND_VAR_CLIENT_REQUEST;
	boost::condition_variable COND_VAR_CLIENT_DONE;

	//Number of connected Agents (entities which respond with a WHOAMI).
	//TODO: We should probably remove this; doesn't one of our data structures contain this?
	size_t numAgents;


	///Set of clients that need their enableRegionSupport() function called. This can only be done once their time tick is over,
	///  so we pend them on this list. The extra weak_ptr shouldn't be a problem; if the object is destroyed before its
	///  call to enableRegionSupport(), it will just be silently dropped.
	std::set< boost::weak_ptr<sim_mob::ClientHandler> > newClientsWaitingOnRegionEnabling;


	///Locks while waiting for agent updates. Unlocks when an Agent informs the Broker that its update loop has completed for this time tick.
	boost::condition_variable COND_VAR_AGENT_DONE;


protected:
	/**
	 * 	configure publisher, message handlers and waiting criteria...
	 */
	void configure();


	/**
	 * There Can be different types of External Entities(clients)
	 * that are configure to be working with Broker. Each one of
	 * them can have a different criteria to Block the broker until
	 * some condition is met(like client's connection to simmobility).
	 * The following method will examine all those Conditions and reports
	 * back if any client is not connected when it was supposed to be connected.
	 */
	bool checkAllBrokerBlockers();

	/**
	 * checks wether an agent(entity) is dead or alive.
	 * Note: this function is not used any more.
	 */
	bool deadEntityCheck(sim_mob::AgentCommUtilityBase * info);

	/**
	 * processes clients requests to be registered with the broker
	 */
	virtual void processClientRegistrationRequests();

	///Helper: Scans a single waitList and processes/removes all entries that it can.
	///The flag "isNs3" indicates an ns-3 simulator; otherwise, it's treated as Android.
	///For now, ns-3 is our only special type.
	void scanAndProcessWaitList(std::queue<ClientWaiting>& waitList, bool isNs3);

	///Relay the COMMSIM_ENABLED message to Agents that requested it.
	void setNewClientProps();

	/**
	 * checks to see if a client has sent all what it had to send during the current tick
	 * Note: The client will send an explicit message stating it is 'done' with whatever
	 * it had to send for the current tick.
	 */
	void waitForClientsDone();


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

	///Called when a new Android client registers.
	virtual void onAndroidClientRegister(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const ClientRegistrationEventArgs& argums);

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

	/**
	 * 	Wait for clients
	 */
	void waitAndAcceptConnections();

	/**
	 * 	wait for the registered agents to complete their tick
	 */
	void waitForAgentsUpdates();

	/**
	 * 	wait for a message from all of the registered the client stating that they are done sending messages for this tick
	 */
	bool allClientsAreDone();

	/**
	 * Deactivate an Agent. This will render it "invalid" for the current time tick.
	 * This function is called when an EVT_CORE_AGENT_DIED event arrives; that occurs at the
	 * start of the time tick, so for the current time tick the agent is considered un-usable
	 * for the purposes of commsim communication.
	 */
	void unRegisterEntity(sim_mob::Agent * agent);

	///Add a ClientRegistration request to the list of waiting Android or NS3 registrations, based on the clientType
	void insertClientWaitingList(std::string clientType, ClientRegistrationRequest request, boost::shared_ptr<sim_mob::ConnectionHandler> existingConn);


public:
	///Callback function executed upon message arrival. Implements the BrokerBase interface.
	virtual void onMessageReceived(boost::shared_ptr<ConnectionHandler>cnnHadler, const BundleHeader& header, std::string message);

	/**
	 * When a new connection is registered (or a NEW_CLIENT message goes out), the
	 *   ConnectionServer sends out a WHOAREYOU message, and then waits for a response.
	 *   Since this response comes in asynchronously, the session pointer is
	 *   pushed to the Broker using this function. It is then paired up with an Agent
	 *   upon receiving a WHOAMI message through the normal channels.
	 *   Implements the BrokerBase interface.
	 * NOTE: For now, this pairing is arbitrary for agents on the same connector. You could
	 *       To ensure that ONLY the agent who received the WHOAREYOU can respond, one
	 *       might add a unique token PER AGENT to the WHOAREYOU message that is then relayed back
	 *       in the WHOAMI (but at the moment this is not necessary. Currently the token is only unique per connection.
	 * THREADING: This function is called directly by thread (via the WhoAreYouProtocol).
	 */
	virtual void insertIntoWaitingOnWHOAMI(const std::string& token, boost::shared_ptr<sim_mob::ConnectionHandler> newConn);

	///Add a ClientHandler to the list of registered Android or NS3 clients, based on the clientType parameter.
	///Implements the BrokerBase interface.
	virtual void insertClientList(std::string clientID, comm::ClientType cType, boost::shared_ptr<sim_mob::ClientHandler>& clientHandler);

	///Retrieve the list of registeredAgents, and the mutex used to lock it. Use this for more extensive modificatoins.
	///Implements the BrokerBase interface.
	virtual AgentsList::type& getRegisteredAgents(AgentsList::Mutex* mutex);

	///Retrieve the Broker's EventPublisher.
	///Implements the BrokerBase interface.
	virtual sim_mob::event::EventPublisher& getPublisher();

	///Retrieve just the size of the registeredAgentsList.
	///Implements the BrokerBase interface.
	virtual size_t getRegisteredAgentsSize() const;

	///Retrieve the list of registered Android clients.
	///Implements the BrokerBase interface.
	virtual const ClientList::Type& getAndroidClientList() const;

	///Insert a message into the appropriate SendBuffer for this ClientHandler.
	///Implements the BrokerBase interface.
	virtual bool insertSendBuffer(boost::shared_ptr<sim_mob::ClientHandler> client, const std::string& message);

	///Searches for an Android client with a specific clientID
	///Implements the BrokerBase interface.
	virtual boost::shared_ptr<sim_mob::ClientHandler> getAndroidClientHandler(std::string clientId) const;

	///Retrieves the ns-3 client, if it exists.
	///Implements the BrokerBase interface.
	virtual boost::shared_ptr<sim_mob::ClientHandler> getNs3ClientHandler() const;

public:
	///Register an Agent with the Broker. This will add it to the registeredAgents list.
	///TODO: This has to do with the AgentCommUtility classes, which need some cleanup.
	void registerEntity(sim_mob::AgentCommUtilityBase* agent);


	//TODO: Not sustainable, but works for now.
	static void SetSingleBroker(Broker* broker);
	static Broker* GetSingleBroker();


	///The Broker performs all updates via Agent::update(), not frame_init() and frame_tick().
	Entity::UpdateStatus update(timeslice now);

	///Implements pure virtual Agent::isNonspatial. Returns true.
	bool isNonspatial();


public:
	//Unused, required overrides.
	void load(const std::map<std::string, std::string>& configProps); ///<Implements pure virtual Agent::load(). Unused.
	bool frame_init(timeslice now); ///<Implements pure virtual Agent::frame_init(). Unused.
	Entity::UpdateStatus frame_tick(timeslice now); ///<Implements pure virtual Agent::frame_tick(). Unused.
	void frame_output(timeslice now); ///<Implements pure virtual Agent::frame_output. Unused.

};

}
