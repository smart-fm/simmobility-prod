/*
 * ConnectionServer.cpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#include "ConnectionServer.hpp"
#include "Session.hpp"
#include "WhoAreYouProtocol.hpp"
#include "logging/Log.hpp"
#include "entities/commsim/broker/Broker.hpp"
namespace sim_mob {

void ConnectionServer::handleNewClient(session_ptr &sess)
{
	//using boost_shared_ptr won't let the protocol to release(i guess).
	//Therefore I used raw pointer. the protocol will delete itself(delete this;)
	WhoAreYouProtocol *registration = new WhoAreYouProtocol(sess,*this);
	registration->start();
}


void ConnectionServer::CreatSocketAndAccept() {
	// Start an accept operation for a new connection.
	std::cout << "Accepting..." <<std::endl; //NOTE: Always print this, even if output is disabled.

	new_sess.reset(new sim_mob::Session(io_service_));
	Print() << "Valid Session[" << new_sess.get() << "]" << std::endl;
	acceptor_.async_accept(new_sess->socket(),
			boost::bind(&ConnectionServer::handle_accept, this,
					boost::asio::placeholders::error, new_sess));
	new_sess.reset();
}

ConnectionServer::ConnectionServer(	sim_mob::Broker &broker_,unsigned short port)
:
		broker(broker_),
		acceptor_(io_service_,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
}

void ConnectionServer::start()
{
	io_service_thread = boost::thread(&ConnectionServer::io_service_run,this);
}

void ConnectionServer::io_service_run()
{
	acceptor_.listen();
	CreatSocketAndAccept();
	io_service_.run();
}
void ConnectionServer::handle_accept(const boost::system::error_code& e, session_ptr &sess) {
	if (!e) {
		std::cout<< "accepted a connection" << std::endl;  //NOTE: Always print this, even if output is disabled.
		handleNewClient(sess);
	} else {
		std::cout<< "refused a connection" << std::endl;  //NOTE: Always print this, even if output is disabled.
		WarnOut("Connection Refused" << std::endl);
	}
	CreatSocketAndAccept();
}
//void ConnectionServer::RequestClientRegistration(unsigned int ID, unsigned int type, session_ptr session_)
void ConnectionServer::RequestClientRegistration(sim_mob::ClientRegistrationRequest &request)
{
//	Print() << "Inside ConnectionServer::RequestClientRegistration" << std::endl;
	unsigned int ID;
	unsigned int type;
	session_ptr session_;
	std::pair<std::string,ClientRegistrationRequest > p(request.client_type, request);
	broker.insertClientWaitingList(p);
}

void ConnectionServer::read_handler(const boost::system::error_code& e, std::string &data, session_ptr &sess) {
	if (!e) {
	} else {
		std::cout << "read Failed" << std::endl;
	}

}

void ConnectionServer::general_send_handler(const boost::system::error_code& e, session_ptr& sess) {
	if (!e) {
//		std::cout << "write Successful" << std::endl;
	} else {
		std::cout << "write Failed:" << e.message() <<  std::endl;
	}

}

ConnectionServer::~ConnectionServer()
{
	acceptor_.cancel();
	acceptor_.close();
	io_service_.stop();
	io_service_thread.join();
}

} /* namespace sim_mob */
