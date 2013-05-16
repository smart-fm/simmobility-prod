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
#include "Session.hpp"
#include <json/json.h>
#include <boost/foreach.hpp>
#include <queue>
#include "../buffer/BufferContainer.hpp"
#include "../message/derived/roadrunner/Serialization.hpp"
namespace sim_mob
{
#define DEFAULT_SERVER_PORT 2013
//typedef boost::shared_ptr<sim_mob::session> session_ptr;


//class Server; //forward declaration
//void clientRegistration_(session_ptr sess, Server *server_);
/***************************************************************************
 *************************        Server       *****************************
 ****************************************************************************/

class Server {

public:
	boost::shared_ptr<boost::mutex> Broker_Client_Mutex;
	boost::shared_ptr<boost::condition_variable> Broker_Client_register;
	boost::mutex server_mutex;
	void clientRegistration_(session_ptr sess);
	void CreatSocketAndAccept();
	Server(	std::queue<std::pair<unsigned int,
			sim_mob::session_ptr > > &clientList_,
			boost::shared_ptr<boost::mutex> Broker_Client_Mutex_,
			boost::shared_ptr<boost::condition_variable> Broker_Client_register_,
			unsigned short port = DEFAULT_SERVER_PORT);
	void start();
	void io_service_run();
	void handle_accept(const boost::system::error_code& e, session_ptr sess);
	void registerClient(unsigned int ID, session_ptr sess);

	void read_handler(const boost::system::error_code& e, std::string &data, session_ptr sess);
	void general_send_handler(const boost::system::error_code& e, session_ptr sess);
	~Server();

	boost::thread io_service_thread; //thread to run the io_service
	boost::asio::io_service io_service_;
private:
	boost::asio::ip::tcp::acceptor acceptor_;
	std::queue<std::pair<unsigned int,sim_mob::session_ptr > > &clientList;
};

class WhoAreYouProtocol
{
public:
	WhoAreYouProtocol(session_ptr sess_, Server *server__);
	void start();
	bool isDone();
	unsigned int getID(std::string ID_);
	std::string ID; //json string containing ID of the client
private:
	session_ptr sess;
	Server *server_;
	bool registerSuccess;
	std::map<unsigned int, session_ptr> clientList;
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
	std::string message;
	//metadata
	unsigned int clientID, agentPtr;
//	boost::tuple<receiveHandler> handler_;
	typedef void (Broker::*BrokerReceiveCallback)(std::string);
	BrokerReceiveCallback &receiveCallBack;
	Broker &theBroker;
public:
	ConnectionHandler(
			session_ptr session_ ,
			Broker& broker,
			BrokerReceiveCallback callback,
			unsigned int clientID_ = 0,
			unsigned long agentPtr_ = 0
			):theBroker(broker), receiveCallBack(callback)

	{
		mySession = session_;
		clientID = clientID_;
		agentPtr = agentPtr_;
	}

	void start()
	{

		Json::Value ready;
		ready["MessageType"] = "Ready";
		Json::FastWriter writer;

		std::string str = writer.write(ready);

		mySession->async_write(str,boost::bind(&ConnectionHandler::readyHandler, this, boost::asio::placeholders::error,str));
	}

	void readyHandler(const boost::system::error_code &e, std::string str)
	{
		if(e)
		{
			std::cerr << "Connection Not Ready [" << e.message() << "] Trying Again" << std::endl;
			mySession->async_write(str,boost::bind(&ConnectionHandler::readyHandler, this, boost::asio::placeholders::error,str));
		}
		else
		{
			//will not pass 'message' variable as argument coz it
			//is global between functions. some function read into it, another function read from it
			mySession->async_read(message,
				boost::bind(&ConnectionHandler::readHandler, this,
						boost::asio::placeholders::error));
		}
	}

	void readHandler(const boost::system::error_code& e) {

		if(e)
		{
			std::cerr << "Read Fail [" << e.message() << "]" << std::endl;
		}
		else
		{
			//call the receive handler in the broker
			CALL_MEMBER_FN(theBroker, receiveCallBack)(message);

			mySession->async_read(message,
							boost::bind(&ConnectionHandler::readHandler, this,
									boost::asio::placeholders::error));
		}
	}

	void send(std::string str)
	{
		mySession->async_write(str,boost::bind(&ConnectionHandler::sendHandler, this, boost::asio::placeholders::error));
	}
	void sendHandler(const boost::system::error_code& e) {
		if(e)
		{
			std::cout << "Write to agent[" << agentPtr << "]  client["  << clientID << "] Failed" << std::endl;
		}
		else
		{
			std::cout << "Write to agent[" << agentPtr << "]  client["  << clientID << "] Success" << std::endl;
		}
	}
};


}//namespace sim_mob
