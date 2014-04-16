//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ConnectionServer.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

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
	//Pend the first client accept.
	creatSocketAndAccept();

	//Create several threads for the io_service to make use of.
	for(unsigned int i=0; i<numThreads; i++) {
	  threads.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
	}

	//NOTE: Always print this, even if output is disabled.
	std::cout << "Accepting clients on " <<numThreads <<" threads.\n";
}


void sim_mob::ConnectionServer::creatSocketAndAccept()
{
	//Make and track a new session pointer.
	boost::shared_ptr<ConnectionHandler> conn(new ConnectionHandler(io_service, broker));
	{
	boost::lock_guard<boost::mutex> lock(knownConnectionsMUTEX);
	knownConnections.push_back(conn);
	}

	//Accept the next connection.
	acceptor.async_accept(conn->socket,
		boost::bind(&ConnectionServer::handle_accept, this, conn, boost::asio::placeholders::error)
	);
}


void sim_mob::ConnectionServer::handle_accept(boost::shared_ptr<ConnectionHandler> conn, const boost::system::error_code& e)
{
	if (e) {
		std::cout<< "Failed to accept connection: " <<e.message() << std::endl;  //NOTE: Always print this, even if output is disabled.
		return;
	}

	//NOTE: Always print this, even if output is disabled.
	std::cout<< "Accepted a connection.\n";

	//Turn off Nagle's algorithm; it's slow on small packets.
	conn->socket.set_option(boost::asio::ip::tcp::no_delay(true));

	//Start listening for the first client message.
	conn->readHeader();

	//Inform the Broker that a new connection is available.
	broker.onNewConnection(conn);

	//Continue; accept the next connection.
	creatSocketAndAccept();
}



