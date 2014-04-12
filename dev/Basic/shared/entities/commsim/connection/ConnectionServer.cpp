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


void sim_mob::ConnectionServer::start()
{
	io_service_thread = boost::thread(&ConnectionServer::io_service_run,this);
}


void sim_mob::ConnectionServer::handleNewClient()
{
	boost::shared_ptr<ConnectionHandler> conn(new ConnectionHandler(newSess, broker));
	WhoAreYouProtocol::QueryAgentAsync(conn, broker);
}


void sim_mob::ConnectionServer::creatSocketAndAccept()
{
	// Start an accept operation for a new connection.
	std::cout << "Accepting..." <<std::endl; //NOTE: Always print this, even if output is disabled.

	//Make and track a new session pointer.
	newSess.reset(new sim_mob::Session(io_service_));
	knownSessions.push_back(newSess);

	//Accept the next connection.
	acceptor_.async_accept(newSess->getSocket(),
		boost::bind(&ConnectionServer::handle_accept, this,
		boost::asio::placeholders::error)
	);
}

void sim_mob::ConnectionServer::io_service_run()
{
	acceptor_.listen();
	creatSocketAndAccept();
	io_service_.run();
}
void sim_mob::ConnectionServer::handle_accept(const boost::system::error_code& e)
{
	if (!e) {
		std::cout<< "accepted a connection" << std::endl;  //NOTE: Always print this, even if output is disabled.
		handleNewClient();
	} else {
		std::cout<< "refused a connection" << std::endl;  //NOTE: Always print this, even if output is disabled.
		WarnOut("Connection Refused" << std::endl);
	}

	//Turn off Nagle's algorithm; it's slow on small packets.
	newSess->getSocket().set_option(boost::asio::ip::tcp::no_delay(true));

	//Continue; accept the next connection.
	creatSocketAndAccept();
}



