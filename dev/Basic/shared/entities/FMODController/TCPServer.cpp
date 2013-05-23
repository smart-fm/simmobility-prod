/*
 * TCPServer.cpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#include "TCPServer.hpp"
#include <iostream>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;
namespace sim_mob {

namespace FMOD
{

TCPServer::TCPServer(boost::asio::io_service& io_service,int port)
		: acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), myPort(port){
	// TODO Auto-generated constructor stub
	start_accept();
}

TCPServer::~TCPServer() {
	// TODO Auto-generated destructor stub
}

void TCPServer::start_accept()
{
	new_connection = TCPSession::create(acceptor_.get_io_service());

	acceptor_.async_accept(new_connection->socket(),
								boost::bind(&TCPServer::handle_accept, this, new_connection,
									boost::asio::placeholders::error));
}

void TCPServer::handle_accept(boost::shared_ptr<TCPSession> new_connection, const boost::system::error_code& error)
{
	start_accept();
}

}
} /* namespace sim_mob */
