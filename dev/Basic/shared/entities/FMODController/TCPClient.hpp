/*
 * TCPSession.hpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#ifndef TCPSESSION_HPP_
#define TCPSESSION_HPP_
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include "MessageQueue.hpp"

namespace sim_mob {

namespace FMOD
{

class TCPServer;
class TCPClient : public boost::enable_shared_from_this<TCPClient> {
public:
	static boost::shared_ptr<TCPClient> create(boost::asio::io_service& io_service, TCPServer* parent=0);

	TCPClient(boost::asio::io_service& io_service, TCPServer* parent=0);
	virtual ~TCPClient();
	void SendMessage(std::string data);
	void SendMessage(MessageList data);
	void Flush();
	MessageList GetMessage();
	bool WaitMessageInBlocking(std::string& msg, int seconds);
	boost::asio::ip::tcp::socket& socket();
	bool ConnectToServer(std::string ip, int port);
	void Stop();

private:
	void handle_write(const boost::system::error_code& error, size_t bytesTransferred);
	void handle_read(const boost::system::error_code& error, size_t bytesTransferred);
	bool sendData();
	bool receiveData();

private:
	boost::asio::ip::tcp::socket socket_;
	MessageQueue msgSendQueue;
	MessageQueue msgReceiveQueue;
	std::string messageSnd;
	boost::array<char, 1> ReceivedBuf;
	TCPServer* parent;

};

typedef boost::shared_ptr<TCPClient> TCPSessionPtr;

}

} /* namespace sim_mob */
#endif /* TCPSESSION_HPP_ */
