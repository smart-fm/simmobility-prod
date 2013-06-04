/*
 * TCPSession.cpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#include "TCPSession.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/thread.hpp>
#include "TCPServer.hpp"

namespace sim_mob {

namespace FMOD
{

TCPSession::TCPSession(boost::asio::io_service& io_service, TCPServer* parentIn):socket_(io_service), parent(parentIn) {
	// TODO Auto-generated constructor stub

}

TCPSession::~TCPSession() {
	// TODO Auto-generated destructor stub
}

boost::shared_ptr<TCPSession> TCPSession::create(boost::asio::io_service& io_service, TCPServer* parent)
{
	return boost::shared_ptr<TCPSession>(new TCPSession(io_service, parent));
}

boost::asio::ip::tcp::socket& TCPSession::socket()
{
	return socket_;
}

void TCPSession::Flush()
{
	sendData();
}


void TCPSession::pushMessage(std::string data)
{
	msgSendQueue.PushMessage(data);
}

void TCPSession::pushMessage(MessageList data)
{
	while(data.size()>0)
	{
		std::string str = data.front();
		data.pop();
		msgSendQueue.PushMessage(str);
	}
}

MessageList TCPSession::popMessage()
{
	MessageList res;
	std::string msg;
	if( bool ret = msgReceiveQueue.PopMessage(msg) )
		res.push(msg);

	return res;
}

void TCPSession::handle_write(const boost::system::error_code& error, size_t bytesTransferred)
{
	if( error == 0 ){
		sendData();
	}
	else{
		 std::cerr<<"end: send error "<<error.message()<<std::endl;
		 if(parent) parent->RemoveAClient(this);
	}
}
void TCPSession::handle_read(const boost::system::error_code& error, size_t bytesTransferred)
{
	if( error == 0 ){
		msgReceiveQueue.PushMessage(ReceivedBuf.data());
		receiveData();
	}
	else{
		 std::cerr<<"end: receive error "<<error.message()<<std::endl;
		 if(parent) parent->RemoveAClient(this);
	}
}

bool TCPSession::ConnectToServer(std::string ip, int port)
{
	bool ret = true;
	boost::asio::ip::tcp::endpoint endpoint( boost::asio::ip::address::from_string(ip.c_str()), port);
	boost::system::error_code ec;
	socket_.connect(endpoint, ec);
	if(ec) {
		std::cerr<<"start: connect error "<<ec.message()<<std::endl;
		ret = false;
	}
	else
	{
		receiveData();
	}
	return ret;
}

void TCPSession::Stop()
{
	socket_.close();
}

bool TCPSession::sendData()
{
	bool ret = msgSendQueue.PopMessage(messageSnd);

	if(!ret) return ret;

	boost::system::error_code err;
	try
	{
		boost::asio::async_write(socket_, boost::asio::buffer(messageSnd),
							  boost::bind(&TCPSession::handle_write,shared_from_this(),
							  boost::asio::placeholders::error,
							  boost::asio::placeholders::bytes_transferred));
		 if(err) {
			 std::cerr<<"start: send error "<<err.message()<<std::endl;
			 if(parent) parent->RemoveAClient(this);
			 return false;
		 }
	}
	catch (std::exception& e)
	{
		std::cerr <<"start: "<< e.what() << std::endl;
		return false;
	}
}
bool TCPSession::receiveData()
{
	boost::system::error_code err;
	try
	{
		boost::asio::async_read(socket_, boost::asio::buffer(ReceivedBuf),
							  boost::bind(&TCPSession::handle_read,shared_from_this(),
							  boost::asio::placeholders::error,
							  boost::asio::placeholders::bytes_transferred));
		 if(err) {
			 std::cerr<<"start: receive error "<<err.message()<<std::endl;
			 if(parent) parent->RemoveAClient(this);
			 return false;
		 }
	}
	catch (std::exception& e)
	{
		std::cerr <<"start: "<< e.what() << std::endl;
		return false;
	}
}


}

} /* namespace sim_mob */
