//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * TCPSession.cpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#include "TCPClient.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/thread.hpp>
#include "TCPServer.hpp"

namespace sim_mob {

namespace FMOD
{

TCPClient::TCPClient(boost::asio::io_service& io_service, TCPServer* parentIn):socket_(io_service), parent(parentIn) {
	// TODO Auto-generated constructor stub

}

TCPClient::~TCPClient() {
	// TODO Auto-generated destructor stub
}

boost::shared_ptr<TCPClient> TCPClient::create(boost::asio::io_service& io_service, TCPServer* parent)
{
	return boost::shared_ptr<TCPClient>(new TCPClient(io_service, parent));
}

boost::asio::ip::tcp::socket& TCPClient::socket()
{
	return socket_;
}

void TCPClient::Flush()
{
	sendData();
}


void TCPClient::SendMessage(std::string data)
{
	msgSendQueue.PushMessage(data);
}

void TCPClient::SendMessage(MessageList data)
{
	while(data.size()>0)
	{
		std::string str = data.front();
		data.pop();
		msgSendQueue.PushMessage(str);
	}
}

MessageList TCPClient::GetMessage()
{
	MessageList res;
	std::string msg;
	if( bool ret = msgReceiveQueue.PopMessage(msg) )
		res.push(msg);

	return res;
}

bool TCPClient::WaitMessageInBlocking(std::string& msg, int seconds)
{
	return msgReceiveQueue.WaitPopMessage(msg, seconds);
}

void TCPClient::handle_write(const boost::system::error_code& error, size_t bytesTransferred)
{
	if( error == 0 ){
		sendData();
	}
	else{
		 std::cerr<<"end: send error "<<error.message()<<std::endl;
		 if(parent) parent->RemoveAClient(this);
	}
}
void TCPClient::handle_read(const boost::system::error_code& error, size_t bytesTransferred)
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

bool TCPClient::ConnectToServer(std::string ip, int port)
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

void TCPClient::Stop()
{
	socket_.close();
}

bool TCPClient::sendData()
{
	bool ret = msgSendQueue.PopMessage(messageSnd);

	if(!ret) return ret;

	boost::system::error_code err;
	try
	{
		boost::asio::async_write(socket_, boost::asio::buffer(messageSnd),
							  boost::bind(&TCPClient::handle_write,shared_from_this(),
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

	return true;
}
bool TCPClient::receiveData()
{
	boost::system::error_code err;
	try
	{
		boost::asio::async_read(socket_, boost::asio::buffer(ReceivedBuf),
							  boost::bind(&TCPClient::handle_read,shared_from_this(),
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

	return true;
}


}

} /* namespace sim_mob */
