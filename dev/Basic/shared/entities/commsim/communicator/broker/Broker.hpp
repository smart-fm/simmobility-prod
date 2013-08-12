#pragma once
#include "entities/Agent.hpp"
#include "entities/commsim/communicator/client-registration/base/ClientRegistration.hpp"
#include "entities/commsim/communicator/service/services.hpp"
#include "entities/commsim/communicator/message/base/MessageQueue.hpp"
#include "entities/commsim/communicator/client-registration/base/ClientRegistrationFactory.hpp"
#include "entities/commsim/communicator/buffer/BufferContainer.hpp"

//external libraries
#include <boost/thread/condition_variable.hpp>

namespace sim_mob
{
//Forward Declarations
template <class RET,class MSG>
class MessageFactory;
template<class T>
class Message;
template<class T>
class AgentCommUtility;
class Publisher;
class ConnectionHandler;
class ConnectionServer;
class ClientHandler;

template<class MSG_TYPE>
struct AgentsMap
{
	typedef boost::unordered_map<const sim_mob::Agent *, AgentCommUtility<MSG_TYPE>* > type;
	typedef typename boost::unordered_map<const sim_mob::Agent *, AgentCommUtility<MSG_TYPE>* >::iterator iterator;
	typedef std::pair<const sim_mob::Agent *, AgentCommUtility<MSG_TYPE>* > pair;
};

 //since we have not created the original key/values, we wont use shared_ptr to avoid crashing
struct MessageElement{
typedef boost::tuple<boost::shared_ptr<sim_mob::ConnectionHandler>, sim_mob::msg_ptr > type;
};

struct MessageFactories{
typedef std::map<unsigned int ,boost::shared_ptr<MessageFactory<std::vector<msg_ptr>&, std::string&> > > type;//<client type, roadrunner message factory>
typedef std::map<unsigned int ,boost::shared_ptr<MessageFactory<std::vector<msg_ptr>&, std::string&> > >::iterator iterator;
typedef std::pair<unsigned int ,boost::shared_ptr<MessageFactory<std::vector<msg_ptr>&, std::string&> > > pair;
};

struct PublisherList{
typedef std::map<sim_mob::SIM_MOB_SERVICE, boost::shared_ptr<sim_mob::Publisher> > type;
typedef std::map<sim_mob::SIM_MOB_SERVICE, boost::shared_ptr<sim_mob::Publisher> >::iterator iterator;
typedef std::pair<sim_mob::SIM_MOB_SERVICE, boost::shared_ptr<sim_mob::Publisher> > pair;
};

struct ClientList{
typedef boost::unordered_map<unsigned int , std::map<std::string , boost::shared_ptr<sim_mob::ClientHandler> > > type; //multimap<client type, map<clientID,clienthandler > >
typedef boost::unordered_map<unsigned int , std::map<std::string , boost::shared_ptr<sim_mob::ClientHandler> > >::iterator iterator;
typedef std::pair<unsigned int , std::map<std::string , boost::shared_ptr<sim_mob::ClientHandler> > > pair;
typedef std::pair<std::string , boost::shared_ptr<sim_mob::ClientHandler> > IdPair;
};

template<class TYPE>
struct SEND_BUFFER {
typedef boost::unordered_map<boost::shared_ptr<sim_mob::ConnectionHandler>, sim_mob::BufferContainer<TYPE> > type;
typedef typename boost::unordered_map<boost::shared_ptr<sim_mob::ConnectionHandler>, sim_mob::BufferContainer<TYPE> >::iterator iterator;
typedef std::pair<boost::shared_ptr<sim_mob::ConnectionHandler>, sim_mob::BufferContainer<TYPE> > pair;
};

class Broker  : public sim_mob::Agent/*, public enable_shared_from_this<Broker>*/ , public event::EventListener//, public sim_mob::MessageReceiver
{
public:
	//for testing purpose
	static int diedAgents;
	static int subscribedAgents;
	explicit Broker(const MutexStrategy& mtxStrat, int id=-1);
	~Broker();
private:
	AgentsMap<std::string>::type registeredAgents;
	ClientWaitList clientRegistrationWaitingList; //<client type, requestform>
	ClientList::type clientList; //key note: there can be one agent associated with multiple clients in this list. why? : coz clients of any type are i this list. and any one has associated itself to this agent for its specific type's reason
	boost::shared_ptr<sim_mob::ConnectionServer> connection;					//accepts, authenticate and registers client connections
	PublisherList::type publishers;
	static const unsigned int MIN_CLIENTS = 1; //minimum number of registered clients(not waiting list)
	static const unsigned int MIN_AGENTS = 1; //minimum number of registered agents
	SEND_BUFFER<Json::Value>::type sendBuffer;
	sim_mob::comm::MessageQueue<sim_mob::MessageElement::type> receiveQueue;
	/*
	 * important note: we put a map of factories for future cases
	 * at present(for example) both client types android_emilator and ns3_simulator
	 * share the same message factory devised for RoadRunner application(RR_Factory):
	 * messageFactories[android] = RR_Factory;
	 * messageFactories[ns3] = RR_Factory;
	 */
	MessageFactories::type messageFactories; //<client type, message factory>
	std::set<const sim_mob::Agent*> duplicateEntityDoneChecker ;
	std::set<boost::shared_ptr<sim_mob::ConnectionHandler> > clientDoneChecker;
	sim_mob::ClientRegistrationFactory clientRegistrationFactory;

	bool deadEntityCheck(sim_mob::AgentCommUtility<std::string> * info);
	void refineSubscriptionList();
	///Returns true if enough subscriptions exist to allow the broker to update.
	bool subscriptionsQualify() const;
	///Returns true if enough clients exist to allow the broker to update.
	bool clientsQualify() const;
	bool brokerCanProceed()const;

	/**
	 * Handles the agent finished event.
	 */
	void OnAgentFinished(event::EventId eventId, event::EventPublisher* sender, const AgentLifeEventArgs& args);

public:

	boost::mutex mutex_client_request;
	boost::mutex mutex_clientList;
	boost::mutex mutex_clientDone;
	boost::condition_variable COND_VAR_CLIENT_REQUEST;
	boost::condition_variable COND_VAR_CLIENT_DONE;

	AgentsMap<std::string>::type & getRegisteredAgents();
	ClientWaitList & getClientWaitingList();
	ClientList::type & getClientList();
	bool getClientHandler(std::string clientId,std::string clientType, boost::shared_ptr<sim_mob::ClientHandler> &output);
	void insertClientList(std::string ,unsigned int , boost::shared_ptr<sim_mob::ClientHandler>&);
	void insertClientWaitingList(std::pair<std::string,ClientRegistrationRequest >);
	PublisherList::type &getPublishers();
	void processClientRegistrationRequests();
	bool insertSendBuffer(boost::shared_ptr<sim_mob::ConnectionHandler>, Json::Value&);
	Entity::UpdateStatus update(timeslice now);
	void removeClient(ClientList::iterator it_erase);
	void waitForClientsDone();
	void cleanup();
	bool allAgentUpdatesDone();
	void messageReceiveCallback(boost::shared_ptr<ConnectionHandler>cnnHadler , std::string message);
	//set to true when there are enough number of subscribers
	//this is used by the Broker to
	//qualifies itself to either
	//-process in/out messages
	//-block the update function and wait for enough number of agents&clients to register
	//-return from update() in order not to block &disturb the simulation
	bool brokerCanTickForward;//used to help deciding whether Broker tick forward or block the simulation


	void processPublishers(timeslice now);
	void sendReadyToReceive();
	void processOutgoingData(timeslice now);
	void processIncomingData(timeslice);

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
//	void assignClient(sim_mob::Entity *agent, std::pair<unsigned int,boost::shared_ptr<sim_mob::Session>> client);

	bool registerEntity(sim_mob::AgentCommUtility<std::string> * );
	void unRegisterEntity(sim_mob::AgentCommUtility<std::string> *value);
	void unRegisterEntity(const sim_mob::Agent * agent);

protected:
	///Wait for clients; return "false" to jump out of the loop.
	bool waitForClientsConnection();
	void waitForAgentsUpdates();
	bool isClientDone(boost::shared_ptr<sim_mob::ClientHandler> &);
	bool allClientsAreDone();

	//Is this Broker currently enabled?
	bool enabled;
private:
	bool firstTime;


};


}
