//simmobility core
#include "message/Message.hpp"
#include "message/MessageReceiver.hpp"
#include "entities/Agent.hpp"
//broker specific
#include "Server/ASIO_Server.hpp"
#include "../../communicator/CommunicationSupport.hpp"
#include "SubscriptionIndex.hpp"
#include "Message/BufferContainer.hpp"
//external libraries


namespace sim_mob
{
//class CommunicationSupport;



class Broker  : public sim_mob::Agent, public sim_mob::MessageReceiver
{
	enum MessageTypes
	{
		 ANNOUNCE = 1,
		 KEY_REQUEST = 2,
		 KEY_SEND = 3
	};
	std::map<std::string, MessageTypes> MessageMap;
	//Broker's main buffers
	std::map<const sim_mob::Agent *, sim_mob::BufferContainer > sendBufferMap; //temporarily used, later the buffer of the agent's communicationsupport will be used
	sim_mob::BufferContainer sendBuffer;//apparently useless
	sim_mob::BufferContainer receiveBuffer;
//	sim_mob::DataContainer trySendBuffer;//send the buffers in batches
	static Broker instance;
	//list of agents willing to participate in communication simulation
	//they are catgorized as those who get a connection and those
	//who are waiting to get one.

	subscriptionC subscriptionList;

	//for find, insert, iteration etc
	subscriberList &subscriberList_;
	agentSubscribers &agentSubscribers_;
	clientSubscribers &clientSubscribers_;

	std::map<const sim_mob::Agent*,subscription > agentList;
	std::map<const sim_mob::Agent*,subscription > agentWaitingList;
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
	boost::shared_ptr<boost::shared_mutex> Broker_Mutex;
	boost::shared_ptr<boost::shared_mutex> Broker_Mutex_Send;
	boost::shared_ptr<boost::shared_mutex> Broker_Mutex_Receive;
	std::vector<boost::shared_ptr<boost::shared_mutex > > mutex_collection;
	subscriptionC &getSubscriptionList();
	explicit Broker(const MutexStrategy& mtxStrat, int id=-1);
	~Broker();
	void preparePerTickData(timeslice now);
	void processIncomingData(timeslice);
	bool handleANNOUNCE(std::string);
	bool handleKEY_REQUEST(std::string data);
	bool handleKEY_SEND(std::string data);
	Entity::UpdateStatus update(timeslice now);
	void processOutgoingData(timeslice now);
	void unicast(const sim_mob::Agent *, std::string);
	void load(const std::map<std::string, std::string>& configProps){};
	bool frame_init(timeslice now){};
	Entity::UpdateStatus frame_tick(timeslice now){};
	void frame_output(timeslice now){};
	//assign a client from clientList to an agent in the agentList
//	void assignClient(sim_mob::Entity *agent, std::pair<unsigned int,session_ptr> client);
	void handleReceiveMessage(std::string);
	std::vector< boost::shared_ptr<boost::shared_mutex> > subscribeEntity(sim_mob::CommunicationSupport&);
	static Broker& GetInstance() { return Broker::instance; }
	bool isNonspatial(){};
};


}
