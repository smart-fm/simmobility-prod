#include "message/Message.hpp"
#include "entities/Agent.hpp"
#include "Server/ASIO_Server.hpp"
//#include "../communication/Communication.hpp"
namespace sim_mob
{
class CommunicationSupport;
struct subscription
{
	subscription(sim_mob::CommunicationSupport* cs) :
			 connected(false) {
		CommunicationSupport_.reset(cs);
	}

	boost::shared_ptr<sim_mob::CommunicationSupport> CommunicationSupport_;
	session_ptr sess;
	unsigned int clientID;
	bool connected;
};
class Broker  : public sim_mob::Agent
{

	boost::thread io_service_thread;
	static Broker instance;
//	sim_mob::Communication commImpl;
	sim_mob::server server_;

	sim_mob::Message iMessages;
	std::map<const sim_mob::Entity*,subscription > agentList;
	std::map<const sim_mob::Entity*,subscription > agentWaitingList;

	std::queue<std::pair<unsigned int,sim_mob::session_ptr > >clientList;
	boost::asio::io_service io_service_;
	void io_service_run(boost::asio::io_service & );

public:
	boost::shared_mutex *Communicator_Mutex;
	boost::shared_mutex *Communicator_Mutex_Send;
	boost::shared_mutex *Communicator_Mutex_Receive;
	std::vector<boost::shared_mutex*> mutex_collection;

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
