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
#include "entities/commsim/communicator/broker/Broker.hpp"
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
	std::cout << "Accepting..." << std::endl;
//	sim_mob::session_ptr new_sess;
	new_sess.reset(new sim_mob::Session(io_service_));
	Print()<< "new_sess.use_count()= " << new_sess.use_count() << std::endl;
	acceptor_.async_accept(new_sess->socket(),
			boost::bind(&ConnectionServer::handle_accept, this,
					boost::asio::placeholders::error, new_sess));
	new_sess.reset();
	Print()<< "new_sess.use_count()= " << new_sess.use_count() << std::endl;
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

		Print()<< "sess.use_count()= " << sess.use_count() << std::endl;
		std::cout << "Connection Accepted" << std::endl;
		handleNewClient(sess);
	}
	else
	{
		std::cout << "Connection Refused" << std::endl;
	}
	CreatSocketAndAccept();
}
//void ConnectionServer::RequestClientRegistration(unsigned int ID, unsigned int type, session_ptr session_)
void ConnectionServer::RequestClientRegistration(sim_mob::ClientRegistrationRequest &request)
{
	Print() << "Inside ConnectionServer::RequestClientRegistration" << std::endl;
	unsigned int ID;
	unsigned int type;
	session_ptr session_;
	std::pair<std::string,ClientRegistrationRequest > p(request.client_type, request);
	broker.insertClientWaitingList(p);
//	std::cout << " RequestClientRegistration Done, returning" << std::endl;
}

void ConnectionServer::read_handler(const boost::system::error_code& e, std::string &data, session_ptr &sess) {
	if (!e) {
		std::cout << "read Successful" << std::endl;
	} else {
		std::cout << "read Failed" << std::endl;
	}

}

void ConnectionServer::general_send_handler(const boost::system::error_code& e, session_ptr& sess) {
	if (!e) {
		std::cout << "write Successful" << std::endl;
	} else {
		std::cout << "write Failed:" << e.message() <<  std::endl;
	}

}

ConnectionServer::~ConnectionServer()
{
	io_service_.stop();
	io_service_thread.join();
}

} /* namespace sim_mob */
