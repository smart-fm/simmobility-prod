//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ConnectionServer.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#pragma once

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include<queue>

#include <entities/commsim/client/base/ClientRegistration.hpp>
#include <entities/commsim/connection/Session.hpp>
namespace sim_mob {
class Broker;

class ConnectionServer {
public:
	ConnectionServer(sim_mob::Broker &broker_,unsigned short port = DEFAULT_SERVER_PORT);
	~ConnectionServer();

	///This function is called exactly once for every new incoming connection (session).
	void handleNewClient(boost::shared_ptr<sim_mob::Session> &sess);

	///This is used to loop accepting connections.
	void CreatSocketAndAccept();

	void start();
	void io_service_run();
	void handle_accept(const boost::system::error_code& e, boost::shared_ptr<sim_mob::Session> &sess);
	void RequestClientRegistration(const sim_mob::ClientRegistrationRequest& request, boost::shared_ptr<sim_mob::ConnectionHandler> existingConn);
	void read_handler(const boost::system::error_code& e, std::string &data, boost::shared_ptr<sim_mob::Session>& sess);
	void general_send_handler(const boost::system::error_code& e, boost::shared_ptr<sim_mob::Session>& sess);

	boost::thread io_service_thread; //thread to run the io_service
	boost::asio::io_service io_service_;
private:
	sim_mob::session_ptr new_sess;
	const static unsigned int DEFAULT_SERVER_PORT = 6745;
	boost::asio::ip::tcp::acceptor acceptor_;
	sim_mob::Broker &broker;
};

} /* namespace sim_mob */
