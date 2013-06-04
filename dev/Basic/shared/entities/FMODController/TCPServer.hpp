/*
 * TCPServer.hpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#ifndef TCPSERVER_HPP_
#define TCPSERVER_HPP_
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include "TCPSession.hpp"
#include "vector"

namespace sim_mob {

namespace FMOD
{

class TCPServer {
public:
	TCPServer(boost::asio::io_service& io_service,int port);
	virtual ~TCPServer();
public:
	bool IsClientConnect() { return connectionList.size()>0;}
	void InsertAClient(boost::shared_ptr<TCPSession> connection);
	void RemoveAClient(TCPSession* connection);
	void Close();

private:
	std::vector<TCPSessionPtr> connectionList;
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::shared_mutex mutex;
	int myPort;

private:
	void StartAccept();
	void handle_accept(boost::shared_ptr<TCPSession> connection, const boost::system::error_code& error);
};

typedef boost::shared_ptr<TCPServer> TCPServerPtr;

}

} /* namespace sim_mob */
#endif /* TCPSERVER_HPP_ */
