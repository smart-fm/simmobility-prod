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

#include <entities/commsim/communicator/client-registration/base/ClientRegistration.hpp>
#include <entities/commsim/communicator/connection/Session.hpp>
namespace sim_mob {
//class Session;
class Broker;

class ConnectionServer {
	sim_mob::session_ptr new_sess;
	public:
//		boost::shared_ptr<boost::mutex> Broker_Client_Mutex;
//		boost::shared_ptr<boost::condition_variable> COND_VAR_CLIENT_REQUEST;
//		boost::mutex server_mutex;
		void handleNewClient(boost::shared_ptr<sim_mob::Session> &sess);
		void CreatSocketAndAccept();
//		sim_mob::ClientRegistrationRequest t;
//		std::queue<boost::tuple<unsigned int,sim_mob::ClientRegistrationRequest > > tt;
		ConnectionServer(sim_mob::Broker &broker_,unsigned short port = DEFAULT_SERVER_PORT);
		void start();
		void io_service_run();
		void handle_accept(const boost::system::error_code& e, boost::shared_ptr<sim_mob::Session> &sess);
		void RequestClientRegistration(sim_mob::ClientRegistrationRequest &request);

		void read_handler(const boost::system::error_code& e, std::string &data, boost::shared_ptr<sim_mob::Session>& sess);
		void general_send_handler(const boost::system::error_code& e, boost::shared_ptr<sim_mob::Session>& sess);
		~ConnectionServer();

		boost::thread io_service_thread; //thread to run the io_service
		boost::asio::io_service io_service_;
	private:
		const static unsigned int DEFAULT_SERVER_PORT = 6745;
		boost::asio::ip::tcp::acceptor acceptor_;
		sim_mob::Broker &broker;
};

} /* namespace sim_mob */
