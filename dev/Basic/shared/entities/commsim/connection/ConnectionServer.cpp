//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ConnectionServer.cpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#include "ConnectionServer.hpp"
#include "entities/commsim/connection/Session.hpp"
#include "entities/commsim/connection/WhoAreYouProtocol.hpp"
#include "logging/Log.hpp"
#include "entities/commsim/Broker.hpp"

using namespace sim_mob;

sim_mob::ConnectionServer::ConnectionServer(	sim_mob::Broker &broker_,unsigned short port) :
	broker(broker_),
	acceptor_(io_service_,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
}

sim_mob::ConnectionServer::~ConnectionServer()
{
	acceptor_.cancel();
	acceptor_.close();
	io_service_.stop();
	io_service_thread.join();
}


void sim_mob::ConnectionServer::handleNewClient(session_ptr &sess)
{
	//using boost_shared_ptr won't let the protocol to release(i guess).
	//Therefore I used raw pointer. the protocol will delete itself(delete this;)
	WhoAreYouProtocol *registration = new WhoAreYouProtocol(sess,*this, broker);
	registration->queryAgentAsync();

	//NOTE: Since a connection via the ConnectionServer *always* represents a new session, we
	//      need to bind to the normal message loop here (through broker?)
	//TODO: Basically, the WhoAreYouProtocol would call this:
	//      server.RequestClientRegistration(request);
	//      ...where "server" is this server. This would add the session to the Broker's
	//      client list, and it would be processed on the next turn tick.
	//TODO: The Broker would then call AndroidClientRegistration::handle(), which calls its own
	//      internal method, makeClientHandler(). This function calls the Broker's getMessageReceiveCallBack()
	//      function, which returns a function pointer to Broker::messageReceiveCallback(). This function
	//      is where we will need to handle agent/message multiplexing. (Since all Messages go through the Broker,
	//      this is not actually so challenging).
	//TODO: The main problem here is that AndroidClientRegistration::handle() does BOTH agent assignment and
	//      boost::reader stuff. So we need to decouple it.
}


void sim_mob::ConnectionServer::CreatSocketAndAccept()
{
	// Start an accept operation for a new connection.
	std::cout << "Accepting..." <<std::endl; //NOTE: Always print this, even if output is disabled.

	new_sess.reset(new sim_mob::Session(io_service_));
	Print() << "Valid Session[" << new_sess.get() << "]" << std::endl;
	acceptor_.async_accept(new_sess->socket(),
			boost::bind(&ConnectionServer::handle_accept, this,
					boost::asio::placeholders::error, new_sess));
	new_sess.reset();
}

void sim_mob::ConnectionServer::start()
{
	io_service_thread = boost::thread(&ConnectionServer::io_service_run,this);
}

void sim_mob::ConnectionServer::io_service_run()
{
	acceptor_.listen();
	CreatSocketAndAccept();
	io_service_.run();
}
void sim_mob::ConnectionServer::handle_accept(const boost::system::error_code& e, session_ptr &sess)
{
	if (!e) {
		std::cout<< "accepted a connection" << std::endl;  //NOTE: Always print this, even if output is disabled.
		handleNewClient(sess);
	} else {
		std::cout<< "refused a connection" << std::endl;  //NOTE: Always print this, even if output is disabled.
		WarnOut("Connection Refused" << std::endl);
	}
	CreatSocketAndAccept();
}

void sim_mob::ConnectionServer::RequestClientRegistration(const sim_mob::ClientRegistrationRequest& request)
{
	std::pair<std::string,ClientRegistrationRequest > p(request.client_type, request);
	broker.insertClientWaitingList(p);
}

void sim_mob::ConnectionServer::read_handler(const boost::system::error_code& e, std::string &data, session_ptr &sess)
{
	if (e) {
		Warn()<<"read Failed\n";
	}

}

void sim_mob::ConnectionServer::general_send_handler(const boost::system::error_code& e, session_ptr& sess)
{
	if (e) {
		Warn() <<"write Failed:" << e.message() <<  std::endl;
	}
}

