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

#include <boost/shared_ptr.hpp>

#include "entities/commsim/connection/Session.hpp"
#include "entities/commsim/connection/ConnectionHandler.hpp"
#include "entities/commsim/connection/WhoAreYouProtocol.hpp"
#include "logging/Log.hpp"
#include "entities/commsim/broker/Broker.hpp"

using namespace sim_mob;

sim_mob::ConnectionServer::ConnectionServer(sim_mob::BrokerBase& broker, unsigned short port) :
	broker(broker),
	acceptor(io_service,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
}

sim_mob::ConnectionServer::~ConnectionServer()
{
	acceptor.cancel();
	io_service.stop();
	threads.join_all();
}


void sim_mob::ConnectionServer::start(unsigned int numThreads)
{
	//TODO: We can easily remove this later.
	if (numThreads!=1) { throw std::runtime_error("ConnectionServer can only take 1 thread."); }

	//Pend the first client accept.
	creatSocketAndAccept();

	//Create several threads for the io_service to make use of.
	for(unsigned int i=0; i<numThreads; i++) {
	  threads.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
	}
}


void sim_mob::ConnectionServer::creatSocketAndAccept()
{
	// Start an accept operation for a new connection.
	std::cout << "Accepting..." <<std::endl; //NOTE: Always print this, even if output is disabled.

	//Make and track a new session pointer.
	knownSessions.push_back(session_ptr(new sim_mob::Session(io_service)));

	//Accept the next connection.
	acceptor.async_accept(knownSessions.back()->getSocket(),
		boost::bind(&ConnectionServer::handle_accept, this,
		boost::asio::placeholders::error)
	);
}


void sim_mob::ConnectionServer::handle_accept(const boost::system::error_code& e)
{
	if (e) {
		std::cout<< "Failed to accept connection: " <<e.message() << std::endl;  //NOTE: Always print this, even if output is disabled.
		return;
	}

	//Otherwise, handle the new connection and then continue the cycle.
	std::cout<< "Accepted a connection\n";  //NOTE: Always print this, even if output is disabled.

	//Turn off Nagle's algorithm; it's slow on small packets.
	knownSessions.back()->getSocket().set_option(boost::asio::ip::tcp::no_delay(true));

	//Ask the client to identify itself (this will also save the ConnectionHandler in the Broker).
	boost::shared_ptr<ConnectionHandler> conn(new ConnectionHandler(knownSessions.back(), broker));
	WhoAreYouProtocol::QueryAgentAsync(conn, broker);

	//Continue; accept the next connection.
	creatSocketAndAccept();
}



