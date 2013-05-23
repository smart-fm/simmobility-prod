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
#include "MessageQueue.hpp"

namespace sim_mob {

namespace FMOD
{

class TCPSession : public boost::enable_shared_from_this<TCPSession> {
public:
	static boost::shared_ptr<TCPSession> create(boost::asio::io_service& io_service);

	TCPSession(boost::asio::io_service& io_service);
	virtual ~TCPSession();
	void pushMessage(std::string data);
	MessageList popMessage();
	boost::asio::ip::tcp::socket& socket();

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

};

}

} /* namespace sim_mob */
#endif /* TCPSESSION_HPP_ */
