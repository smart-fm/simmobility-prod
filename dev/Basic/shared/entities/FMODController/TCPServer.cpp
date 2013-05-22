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

}

TCPServer::~TCPServer() {
	// TODO Auto-generated destructor stub
}

}
} /* namespace sim_mob */
