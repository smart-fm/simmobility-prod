//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * FmodClient.cpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#include "FMOD_Client.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/thread.hpp>

namespace sim_mob {

namespace FMOD
{

FMOD_Client::FMOD_Client(boost::asio::io_service& ioService):socket_(ioService) {
	// TODO Auto-generated constructor stub

}

FMOD_Client::~FMOD_Client() {
	// TODO Auto-generated destructor stub
}

boost::shared_ptr<FMOD_Client> FMOD_Client::create(boost::asio::io_service& ioService)
{
	return boost::shared_ptr<FMOD_Client>(new FMOD_Client(ioService));
}

boost::asio::ip::tcp::socket& FMOD_Client::socket()
{
	return socket_;
}

void FMOD_Client::flush()
{
	sendData();
}


void FMOD_Client::sendMessage(std::string& data)
{
	msgSendQueue.PushMessage(data);
}

void FMOD_Client::sendMessage(MessageList& data)
{
	while(data.size()>0)
	{
		std::string str = data.front();
		data.pop();
		msgSendQueue.PushMessage(str);
	}
}

MessageList FMOD_Client::getMessage()
{
	MessageList res;
	std::string msg;
	if( bool ret = msgReceiveQueue.PopMessage(msg) ){
		res.push(msg);
	}

	return res;
}

bool FMOD_Client::waitMessageInBlocking(std::string& msg, int seconds)
{
	return msgReceiveQueue.WaitPopMessage(msg, seconds);
}

void FMOD_Client::handleWrite(const boost::system::error_code& error, size_t bytesTransferred)
{
	if( error == 0 ){
		sendData();
	}
	else{
		 std::cerr<<"end: send error "<<error.message()<<std::endl;
	}
}
void FMOD_Client::handleRead(const boost::system::error_code& error, size_t bytesTransferred)
{
	if( error == 0 ){
		msgReceiveQueue.PushMessage(ReceivedBuf.data());
		receiveData();
	}
	else{
		 std::cerr<<"end: receive error "<<error.message()<<std::endl;
	}
}

bool FMOD_Client::connectToServer(std::string& ip, int port)
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

void FMOD_Client::stop()
{
	socket_.close();
}

bool FMOD_Client::sendData()
{
	bool ret = msgSendQueue.PopMessage(messageSnd);

	if(!ret){
		return ret;
	}

	boost::system::error_code err;
	try
	{
		boost::asio::async_write(socket_, boost::asio::buffer(messageSnd),
							  boost::bind(&FMOD_Client::handleWrite,shared_from_this(),
							  boost::asio::placeholders::error,
							  boost::asio::placeholders::bytes_transferred));
		 if(err) {
			 std::cerr<<"start: send error "<<err.message()<<std::endl;
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
bool FMOD_Client::receiveData()
{
	boost::system::error_code err;
	try
	{
		boost::asio::async_read(socket_, boost::asio::buffer(ReceivedBuf),
							  boost::bind(&FMOD_Client::handleRead,shared_from_this(),
							  boost::asio::placeholders::error,
							  boost::asio::placeholders::bytes_transferred));
		 if(err) {
			 std::cerr<<"start: receive error "<<err.message()<<std::endl;
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
