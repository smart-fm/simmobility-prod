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
class JCommunicationSupport;

class Publisher;
class ConnectionHandler;
class ConnectionServer;
class ClientHandler;

typedef std::map<const sim_mob::Agent *, JCommunicationSupport<std::string>* > AgentsMap; //since we have not created the original key/values, we wont use shared_ptr to avoid crashing
typedef boost::tuple<boost::shared_ptr<sim_mob::ConnectionHandler>, std::string> DataElement; //<sending agent, connectionHandler-containing socket, data>
typedef boost::tuple<boost::shared_ptr<sim_mob::ConnectionHandler>, sim_mob::msg_ptr > MessageElement;
typedef std::map<unsigned int ,boost::shared_ptr<MessageFactory<std::vector<msg_ptr>&, std::string&> > >MessageFactories;//<client type, roadrunner message factory>
typedef std::map<sim_mob::SIM_MOB_SERVICE, boost::shared_ptr<sim_mob::Publisher> > PublisherList;
typedef std::multimap<unsigned int , boost::shared_ptr<sim_mob::ClientHandler> > ClientList; //<client type,clienthandler >

static DataElement makeDataElement(boost::shared_ptr<sim_mob::ConnectionHandler> socket ,std::string &str )
{
	return boost::make_tuple(socket, str);
}


class Broker  : public sim_mob::Agent//, public sim_mob::MessageReceiver
{
public:
	explicit Broker(const MutexStrategy& mtxStrat, int id=-1);
	~Broker();
private:
	AgentsMap registeredAgents;
	ClientWaitList clientRegistrationWaitingList; //<client type, requestform>
	ClientList clientList; //key note: there can be one agent associated with multiple clients in this list. why? : coz clients of any type are i this list. and any one has associated itself to this agent for its specific type's reason
	boost::shared_ptr<sim_mob::ConnectionServer> connection;					//accepts, authenticate and registers client connections
	PublisherList publishers;
	static const unsigned int MIN_CLIENTS = 1; //minimum number of registered clients(not waiting list)
	static const unsigned int MIN_AGENTS = 1; //minimum number of registered agents
	sim_mob::BufferContainer<sim_mob::DataElement> sendBuffer;//apparently useless for this demo
	sim_mob::comm::MessageQueue<sim_mob::MessageElement> receiveQueue;
	/*
	 * important note: we put a map of factories for future cases
	 * at present(for example) both client types android_emilator and ns3_simulator
	 * share the same message factory devised for RoadRunner application(RR_Factory):
	 * messageFactories[android] = RR_Factory;
	 * messageFactories[ns3] = RR_Factory;
	 */
	MessageFactories messageFactories; //<client type, message factory>
	std::set<const sim_mob::Agent*> duplicateEntityDoneChecker ;
	sim_mob::ClientRegistrationFactory clientRegistrationFactory;

	bool deadEntityCheck(sim_mob::JCommunicationSupport<std::string> * info);
	void refineSubscriptionList();
	///Returns true if enough subscriptions exist to allow the broker to update.
	bool subscriptionsQualify() const;
	///Returns true if enough clients exist to allow the broker to update.
	bool clientsQualify() const;
	bool brokerCanProceed()const;

public:
	boost::shared_ptr<boost::mutex> Broker_Client_Mutex;
	boost::shared_ptr<boost::shared_mutex> Broker_Mutex;
	boost::shared_ptr<boost::shared_mutex> Broker_Mutex_Send;
	boost::shared_ptr<boost::shared_mutex> Broker_Mutex_Receive;
	std::vector<boost::shared_ptr<boost::shared_mutex > > mutex_collection;
	boost::shared_ptr<boost::condition_variable> COND_VAR_CLIENT_REQUEST;

	AgentsMap & getRegisteredAgents();
	ClientWaitList & getClientWaitingList();
	ClientList & getClientList();
	PublisherList &getPublishers();
	void processClientRegistrationRequests();
	void insertSendBuffer(DataElement&);
	Entity::UpdateStatus update(timeslice now);
	bool allAgentUpdatesDone();
	void messageReceiveCallback(boost::shared_ptr<ConnectionHandler>cnnHadler , std::string message);

	boost::shared_ptr<boost::shared_mutex> getBrokerMutex();
	boost::shared_ptr<boost::shared_mutex> getBrokerMutexSend();
	boost::shared_ptr<boost::shared_mutex> getBrokerMutexReceive();
	std::vector<boost::shared_ptr<boost::shared_mutex > > & getBrokerMutexCollection();
	boost::shared_ptr<boost::mutex> getBrokerClientMutex();
	//set to true when there are enough number of subscribers
	//this is used by the Broker to
	//qualifies itself to either
	//-process in/out messages
	//-block the update function and wait for enough number of agents&clients to register
	//-return from update() in order not to block &disturb the simulation
	bool brokerCanTickForward;//used to help deciding whether Broker tick forward or block the simulation


	void processPublishers(timeslice now);
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

	bool registerEntity(sim_mob::JCommunicationSupport<std::string> * );
	void unRegisterEntity(sim_mob::JCommunicationSupport<std::string> *value);
	void unRegisterEntity(const sim_mob::Agent * agent);

protected:
	///Wait for clients; return "false" to jump out of the loop.
	bool waitForClients();
	void waitForUpdates();

	//Is this Broker currently enabled?
	bool enabled;


};


}
