//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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
	StartAccept();
}

TCPServer::~TCPServer() {
	// TODO Auto-generated destructor stub
}

void TCPServer::Close()
{
	acceptor_.close();
}


void TCPServer::StartAccept()
{
	TCPSessionPtr connection = TCPClient::create(acceptor_.get_io_service());

	acceptor_.async_accept(connection->socket(),
						boost::bind(&TCPServer::handle_accept, this, connection, boost::asio::placeholders::error));
}

void TCPServer::handle_accept(boost::shared_ptr<TCPClient> connection, const boost::system::error_code& error)
{
	if( error== 0)
	{
		InsertAClient(connection);
		StartAccept();
	}
}

void TCPServer::InsertAClient(boost::shared_ptr<TCPClient> connection)
{
	//boost::unique_lock< boost::shared_mutex > lock(mutex);
	connectionList.push_back(connection);
}
void TCPServer::RemoveAClient(TCPClient* connection)
{
	//boost::unique_lock< boost::shared_mutex > lock(mutex);
	std::vector<TCPSessionPtr>::iterator it;
	for(it=connectionList.begin(); it!=connectionList.end(); it++)
	{
		if((*it).get() == connection ){
			connectionList.erase(it);
			break;
		}
	}
}


}
} /* namespace sim_mob */
