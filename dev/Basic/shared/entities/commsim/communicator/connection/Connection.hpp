#pragma once
#include <boost/thread/locks.hpp>
#include "boost/thread/shared_mutex.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <time.h>
#include <json/json.h>
#include <boost/foreach.hpp>
#include <queue>

#include "Session.hpp"
#include "entities/commsim/communicator/buffer/BufferContainer.hpp"
#include "entities/commsim/communicator/message/derived/roadrunner/Serialization.hpp"
namespace sim_mob
{
//typedef boost::shared_ptr<sim_mob::session> session_ptr;


//class ConnectionServer; //forward declaration
//void clientRegistration_(session_ptr sess, ConnectionServer *server_);
/***************************************************************************
 *************************        ConnectionServer       *****************************
 ****************************************************************************/

class ConnectionServer {

public:
	boost::shared_ptr<boost::mutex> Broker_Client_Mutex;
	boost::shared_ptr<boost::condition_variable> COND_VAR_CLIENT_REQUEST;
	boost::mutex server_mutex;
	void handleNewClient(session_ptr sess);
	void CreatSocketAndAccept();
	ConnectionServer(	std::queue<boost::tuple<unsigned int,ClientRegistrationRequest > > &clientRegistrationWaitingList_,
			boost::shared_ptr<boost::mutex> Broker_Client_Mutex_,
			boost::shared_ptr<boost::condition_variable> COND_VAR_CLIENT_REQUEST_,
			unsigned short port = DEFAULT_SERVER_PORT);
	void start();
	void io_service_run();
	void handle_accept(const boost::system::error_code& e, session_ptr sess);
	void RequestClientRegistration(sim_mob::ClientRegistrationRequest request);

	void read_handler(const boost::system::error_code& e, std::string &data, session_ptr sess);
	void general_send_handler(const boost::system::error_code& e, session_ptr sess);
	~ConnectionServer();

	boost::thread io_service_thread; //thread to run the io_service
	boost::asio::io_service io_service_;
private:
	const static unsigned int DEFAULT_SERVER_PORT = 6745;

	boost::asio::ip::tcp::acceptor acceptor_;
	ClientWaitList &clientRegistrationWaitingList; //<client type,registrationrequestform >
};

class WhoAreYouProtocol
{
public:
	WhoAreYouProtocol(session_ptr sess_, ConnectionServer *server__);
	void start();
	bool isDone();
	void getTypeAndID(std::string &input, unsigned int & out_type, unsigned int & out_ID);
	sim_mob::ClientRegistrationRequest getSubscriptionRequest(std::string, session_ptr);
	std::string response; //json string containing ID & type of the client
private:
	session_ptr sess;
	ConnectionServer *server_;
	bool registerSuccess;
	std::map<unsigned int, session_ptr> clientRegistrationWaitingList;
	void startClientRegistration(session_ptr sess);
	void WhoAreYou_handler(const boost::system::error_code& e,session_ptr sess);
	void WhoAreYou_response_handler(const boost::system::error_code& e, session_ptr sess);
};

//Macro used for callbacks
#define CALL_MEMBER_FN(object, ptrToMember) ((object).*(ptrToMember))

/***************************************************************************
 *************************        ConnectionHandler       *****************************
 ****************************************************************************/
class Broker;
class ConnectionHandler
{
	session_ptr mySession;
//	std::string message;//rudimentary?
	//metadata
	unsigned int clientID, agentPtr, clientType;//some of such data is duplicated in the broker client list entries
//	boost::tuple<receiveHandler> handler_;
	typedef void (Broker::*messageReceiveCallback)(ConnectionHandler&,std::string);
	messageReceiveCallback receiveCallBack;
	Broker &theBroker;
public:
	//NOTE: Passing "callback" by value and then saving it by reference is a bad idea!
	//      For now I've made both work by value; you may need to modify this. ~Seth
	ConnectionHandler(
			session_ptr session_ ,
			Broker& broker,
			messageReceiveCallback callback,
			unsigned int clientID_ = 0,
			unsigned int ClienType_ = 0,
			unsigned long agentPtr_ = 0
			);
	void start();
	void readyHandler(const boost::system::error_code &e, std::string str);
	void readHandler(const boost::system::error_code& e);
	void send(std::string str);
	void sendHandler(const boost::system::error_code& e) ;
};//ConnectionHandler


}//namespace sim_mob
