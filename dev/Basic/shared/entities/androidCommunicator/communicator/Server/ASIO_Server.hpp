#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <time.h>
#include "session.hpp"
#include <jsoncpp/json.h>
#include <boost/foreach.hpp>
#include <queue>
#include "../Message/DataMessage.hpp"
#include "message/MessageReceiver.hpp"
namespace sim_mob
{

typedef boost::shared_ptr<sim_mob::session> session_ptr;
class JsonParser
{

public:
	static unsigned int getID(std::string values)
	{
		Json::Value root;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(values, root, false);
		if(not parsedSuccess)
		{
			std::cout << "Parsing -" << values << "- Failed" << std::endl;
			return 0;
		}

		return root["ID"].asUInt();
	}

	static std::string makeWhoAreYou()
	{
		Json::Value whoAreYou;
		whoAreYou["MessageType"] = "WhoAreYou";
		Json::FastWriter writer;

		std::ostringstream out("");
		return writer.write(whoAreYou);
	}
	static std::string makeTime()
	{
		Json::Value time;
		time["MessageType"] = "TimeData";
		Json::Value breakDown;
		breakDown["hh"] = "12";
		breakDown["mm"] = "13";
		breakDown["ss"] = "14";
		breakDown["ms"] = "15";
		time["TimeData"] = breakDown;
		Json::FastWriter writer;
		return writer.write(time);
	}
	//@originalMessage input
	//@extractedType output
	//@extractedData output
	//@root output
	static bool getMessageTypeAndData(std::string &originalMessage, std::string &extractedType, std::string &extractedData, Json::Value &root_)
	{
		Json::Value root;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(originalMessage, root, false);
		if(not parsedSuccess)
		{
			std::cout << "Parsing [" << originalMessage << "] Failed" << std::endl;
			return false;
		}
		extractedType = root["MessageType"].asString();
		//the rest of the message is actually named after the type name
		extractedData = root[extractedType.c_str()].asString();
		root_ = root;
		return true;
	}
};

class server; //forward declaration
void clientRegistration_(session_ptr sess, server *server_);


class server {

public:
	boost::mutex server_mutex;
	void CreatSocketAndAccept() {
		// Start an accept operation for a new connection.
		std::cout << "Accepting..." << std::endl;
		session_ptr new_sess(new sim_mob::session(io_service_));
		acceptor_.async_accept(new_sess->socket(),
				boost::bind(&server::handle_accept, this,
						boost::asio::placeholders::error, new_sess));
	}

	server(boost::asio::io_service& io_service, std::queue<std::pair<unsigned int,boost::shared_ptr<session> > > &clientList_, unsigned short port = 2013) :
			io_service_(io_service), acceptor_(io_service,
			        boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
			        clientList(clientList_)
	{
	}
	void start()
	{
		acceptor_.listen();
		CreatSocketAndAccept();

	}

	void handle_accept(const boost::system::error_code& e, session_ptr sess) {
		if (!e) {
//			boost::mutex::scoped_lock lock(server_mutex);
			std::cout << "Connection Accepted" << std::endl;
//			boost::thread *t = new boost::thread(clientRegistration_,sess,this);
//			registrationThreads.push_back(t);
			clientRegistration_(sess,this);
			std::cout << "clientRegistration returned" <<   std::endl;
		}
		CreatSocketAndAccept();
	}
	void registerClient(unsigned int ID, session_ptr sess)
	{
//		boost::mutex::scoped_lock lock(server_mutex);//todo remove comment
		std::cout << " registerClient in progress" << std::endl;
		clientList.push(std::make_pair(ID,sess));
		std::cout << " registerClient success, returning" << std::endl;
	}

	void read_handler(const boost::system::error_code& e, std::string &data, session_ptr sess) {
		if (!e) {
			std::cout << "read Successful" << std::endl;
		} else {
			std::cout << "read Failed" << std::endl;
		}

	}

	void general_send_handler(const boost::system::error_code& e, session_ptr sess) {
		if (!e) {
			std::cout << "write Successful" << std::endl;
		} else {
			std::cout << "write Failed:" << e.message() <<  std::endl;
		}

	}
	~server()
	{
	}

private:
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::io_service& io_service_;
//	std::map<unsigned int, session_ptr> clientList;
	std::queue<std::pair<unsigned int,boost::shared_ptr<session> > > &clientList;

//	std::vector<boost::thread *> registrationThreads;
};

class WhoAreYouProtocol
{
public:
	WhoAreYouProtocol(session_ptr sess_, server *server__):sess(sess_),server_(server__),registerSuccess(false){
	}
	void start()
	{
		startClientRegistration(sess);
	}
	bool isDone()
	{
		return registerSuccess;
	}
	unsigned int getID(std::string ID_)//we can declare method without arg also
	{
//		if(!isDone()) return 0;
		return JsonParser::getID(ID_);//u could mention ID instead of ID
	}
	std::string ID; //json string containing ID of the client
private:
	session_ptr sess;
	server *server_;
	bool registerSuccess;
	std::map<unsigned int, session_ptr> clientList;
//	std::string ID; //json string containing ID of the client

	void startClientRegistration(session_ptr sess) {
		std::string str = JsonParser::makeWhoAreYou();
		sess->async_write(str,
				boost::bind(&WhoAreYouProtocol::WhoAreYou_handler, this,
						boost::asio::placeholders::error, sess));
	}
	void WhoAreYou_handler(const boost::system::error_code& e,session_ptr sess) {

		sess->async_read(ID,
				boost::bind(&WhoAreYouProtocol::WhoAreYou_response_handler, this,
						boost::asio::placeholders::error, sess));
	}

	void WhoAreYou_response_handler(const boost::system::error_code& e, session_ptr sess) {
		if(e)
		{
			std::cerr << "WhoAreYou_response_handler Fail [" << e.message() << "]" << std::endl;
		}
		else
		{
	        unsigned int i = getID(ID);
	        std::cout << "ID = " << i << std::endl;
			server_->registerClient(i , sess);
			std::cout << "registering the client Done" << std::endl;
			delete this; //first time in my life! people say it is ok.


		}
	}
};



class ConnectionHandler
{
	session_ptr mySession;
	std::string message;
	sim_mob::MessageReceiver &rHandler;
	//metadata
	unsigned int clientID, agentPtr;
public:
	ConnectionHandler(session_ptr session_, sim_mob::MessageReceiver &rHandler_, unsigned int clientID_ = 0, unsigned int agentPtr_ = 0):rHandler(rHandler_)
	{
		mySession = session_;
		clientID = clientID_;
		agentPtr = agentPtr_;
	}

	void start()
	{

		Json::Value whoAreYou;
		whoAreYou["MessageType"] = "Ready";
		Json::FastWriter writer;

		std::string str = writer.write(whoAreYou);

//		void (ConnectionHandler::*f)(const boost::system::error_code&) = &ConnectionHandler::readyHandler;
		mySession->async_write(str,boost::bind(&ConnectionHandler::readyHandler, this, boost::asio::placeholders::error));

	}


	void readyHandler(const boost::system::error_code &e)
	{
		if(e)
		{
			std::cerr << "Connection Not Ready [" << e.message() << "]" << std::endl;
		}
		else
		{
			//will not pass 'message' variable as argument coz it
			//is global between functions. some function read into it, another function read from it
//			void (ConnectionHandler::*f)(const boost::system::error_code&) = &ConnectionHandler::readHandler;

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
			//After message type is extracted, the
			//rest of the message string is wrapped into a BrokerMessage
			std::string type_, data_;
			Json::Value root;
			if(sim_mob::JsonParser::getMessageTypeAndData(message,type_,data_, root));
			BrokerMessage *message_ = new BrokerMessage(data_, root, this);
			unsigned int type = atoi(type_.c_str());
			rHandler.Post(type, &rHandler, message_);
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
			//good for you!
		}
	}
};


}//namespace sim_mob
