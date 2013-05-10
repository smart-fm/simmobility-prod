#include "Server.hpp"
//testing to see them error free:
//#include "../message/derived/ANNOUNCE_Handler.hpp"
//#include "../message/derived/ANNOUNCE_Message.hpp"
//#include "../message/base/Message.hpp"
//#include "../message/base/Handler.hpp"
//#include "../message/base/HandlerFactory.hpp"
namespace sim_mob
{
void Server::clientRegistration_(session_ptr sess)
{
	//using boost_shared_ptr won't let the protocol to release(i guess).
	//Therefore I used raw pointer. the protocol will delete itself(delete this;)
	WhoAreYouProtocol *registration = new WhoAreYouProtocol(sess,this);
	registration->start();
}


void Server::CreatSocketAndAccept() {
	// Start an accept operation for a new connection.
	std::cout << "Accepting..." << std::endl;
	sim_mob::session_ptr new_sess(new sim_mob::Session(io_service_));
	acceptor_.async_accept(new_sess->socket(),
			boost::bind(&Server::handle_accept, this,
					boost::asio::placeholders::error, new_sess));
}

Server::Server(	std::queue<std::pair<unsigned int,
		sim_mob::session_ptr > > &clientList_,
		boost::shared_ptr<boost::mutex> Broker_Client_Mutex_,
		boost::shared_ptr<boost::condition_variable> Broker_Client_register_,
		unsigned short port)
:
		Broker_Client_Mutex(Broker_Client_Mutex_),
		Broker_Client_register(Broker_Client_register_),
		acceptor_(io_service_,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
		clientList(clientList_)
{

}

void Server::start()
{
	io_service_thread = boost::thread(&Server::io_service_run,this);
}

void Server::io_service_run()
{
	acceptor_.listen();
	CreatSocketAndAccept();
	io_service_.run();
}
void Server::handle_accept(const boost::system::error_code& e, session_ptr sess) {
	if (!e) {
		std::cout << "Connection Accepted" << std::endl;
		clientRegistration_(sess);
	}
	else
	{
		std::cout << "Connection Refused" << std::endl;
	}
	CreatSocketAndAccept();
}
void Server::registerClient(unsigned int ID, session_ptr sess)
{
	boost::unique_lock< boost::mutex > lock(*Broker_Client_Mutex);//todo remove comment
	std::cout << " registerClient in progress" << std::endl;
	clientList.push(std::make_pair(ID,sess));
	Broker_Client_register->notify_one();
	std::cout << " registerClient success, returning" << std::endl;
}

void Server::read_handler(const boost::system::error_code& e, std::string &data, session_ptr sess) {
	if (!e) {
		std::cout << "read Successful" << std::endl;
	} else {
		std::cout << "read Failed" << std::endl;
	}

}

void Server::general_send_handler(const boost::system::error_code& e, session_ptr sess) {
	if (!e) {
		std::cout << "write Successful" << std::endl;
	} else {
		std::cout << "write Failed:" << e.message() <<  std::endl;
	}

}

Server::~Server()
{
	io_service_.stop();
	io_service_thread.join();
}


/*
 * **********************************************
 * *************  WhoAreYouProtocol *************
 * **********************************************
 */

WhoAreYouProtocol::WhoAreYouProtocol(session_ptr sess_, Server *server__):sess(sess_),server_(server__),registerSuccess(false){
	}
	void WhoAreYouProtocol::start()
	{
		startClientRegistration(sess);
	}
	bool WhoAreYouProtocol::isDone()
	{
		return registerSuccess;
	}
	unsigned int WhoAreYouProtocol::getID(std::string ID_)//we can declare method without arg also
	{
		return JsonParser::getID(ID_);//u could mention ID instead of ID
	}

	void WhoAreYouProtocol::startClientRegistration(session_ptr sess) {
		std::string str = JsonParser::makeWhoAreYou();
		std::cout<< " WhoAreYou protocol Sending [" << str << "]" <<  std::endl;
		sess->async_write(str,
				boost::bind(&WhoAreYouProtocol::WhoAreYou_handler, this,
						boost::asio::placeholders::error, sess));
	}
	void WhoAreYouProtocol::WhoAreYou_handler(const boost::system::error_code& e,session_ptr sess) {
		std::cout<< " WhoAreYou_handler readring" << std::endl;
		sess->async_read(ID,
				boost::bind(&WhoAreYouProtocol::WhoAreYou_response_handler, this,
						boost::asio::placeholders::error, sess));
	}

	void WhoAreYouProtocol::WhoAreYou_response_handler(const boost::system::error_code& e, session_ptr sess) {
		if(e)
		{
			std::cerr << "WhoAreYou_response_handler Fail [" << e.message() << "]" << std::endl;
		}
		else
		{
			std::cout<< " WhoAreYou_handler read Done [" << ID << "]"  << std::endl;
	        unsigned int i = getID(ID);
	        std::cout << "ID = " << i << std::endl;
			server_->registerClient(i , sess);
			std::cout << "registering the client Done" << std::endl;
			delete this; //first time in my life! people say it is ok.


		}
	}

};
