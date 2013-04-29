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

	server(boost::asio::io_service& io_service, unsigned short port = 2013) :
			io_service_(io_service), acceptor_(io_service,
			        boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
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
		clientList[ID] = sess;
		std::cout << " registerClient success, returning" << std::endl;
	}
	void sendTime(session_ptr sess)
	{

		std::string str = JsonParser::makeTime();
		sess->async_write(str,
				boost::bind(&server::general_send_handler, this,
						boost::asio::placeholders::error, sess));
	}


	void send(unsigned int receiver, std::string &data) {
		clientList[receiver]->async_write(data,
				boost::bind(&server::write_handler, this,
						boost::asio::placeholders::error,
						boost::ref(clientList[receiver])));
	}

	void write_handler(const boost::system::error_code& e, session_ptr sess) {
		if (!e) {
			std::cout << "Write Successful" << std::endl;
		} else {
			std::cout << "Write Failed" << std::endl;
		}

	}

	void receive(unsigned int receiver, std::string &data) {
		clientList[receiver]->async_read(data,
				boost::bind(&server::read_handler, this,
						boost::asio::placeholders::error, boost::ref(data),
						boost::ref(clientList[receiver])));
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

	const std::map<unsigned int, session_ptr> & getClientList() const {
		return clientList;
	}
	~server()
	{
//		boost::thread *t;
//		BOOST_FOREACH(t , registrationThreads)
//			t->join();
	}

private:
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::io_service& io_service_;
	std::map<unsigned int, session_ptr> clientList;

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

//		  ID.resize(1);
//		  ID = "A";
//		  std::cout << "1.Address of String1 " << &ID << " value[" << ID << "] size " << ID.size() << std::endl;
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
//	        std::string archive_data(&((*ID_)[0]), ID_->size());
//	        std::cout << "WhoAreYou_response_handler gets [" << archive_data << "]" << std::endl;
	        unsigned int i = getID(ID);
	        std::cout << "ID = " << i << std::endl;
			server_->registerClient(i , sess);
			std::cout << "registering the client Done" << std::endl;
			delete this; //first time in my life! people say it is ok.


		}
	}
};



}//namespace sim_mob
