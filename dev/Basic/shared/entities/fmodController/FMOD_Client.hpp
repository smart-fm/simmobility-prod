//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * FmodClient.hpp
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
#include "FMOD_MsgQueue.hpp"

namespace sim_mob {

namespace FMOD
{

/**
  * provide communicate with FMOD simulator
  */
class FMOD_Client : public boost::enable_shared_from_this<FMOD_Client> {
public:

    /**
      * create communication instance
      * @param ioService is IO service in boost library
      * @return a shared pointer to the instance
      */
	static boost::shared_ptr<FMOD_Client> create(boost::asio::io_service& ioService);

	FMOD_Client(boost::asio::io_service& ioService);
	virtual ~FMOD_Client();

    /**
      * send message to FMOD simulator
      * @param data is to store message content
      * @return void
      */
	void sendMessage(std::string data);

    /**
      * send a list of messages to FMOD simulator
      * @param dataList is a list of messages
      * @return void
      */
	void sendMessage(MessageList dataList);

    /**
      * force buffered data to be sent out
       * @return void
      */
	void flush();

    /**
      * retrieve a list of messages from FMOD simulator in non blocking mode
      * @return messages list
      */
	MessageList getMessage();

    /**
      * retrieve a messages from FMOD simulator in blocking mode
      * @return true if recieve a messages successfully
      */
	bool waitMessageInBlocking(std::string& msg, int seconds);

    /**
      * retrieve a socket instance
      * @return the socket
      */
	boost::asio::ip::tcp::socket& socket();

    /**
      * build a connection to server
      * @param ip is FMOD simulator address
      * @param port is FMOD simulator port
      * @return true if connection successfully, otherwise false
      */
	bool connectToServer(std::string ip, int port);

    /**
      * stop a connection to FMOD simulator
      * @return void
      */
	void stop();

private:
    /**
      * writing callback function for boost IO service
      * @param error is error code returned from boost IO service
      * @param bytesTransferred is to record transfer size
      * @return void
      */
	void handleWrite(const boost::system::error_code& error, size_t bytesTransferred);

    /**
      * reading callback function for boost IO service
      * @param error is error code returned from boost IO service
      * @param bytesTransferred is to record transfer size
      * @return void
      */
	void handleRead(const boost::system::error_code& error, size_t bytesTransferred);

    /**
      * start sending data to FMOD simulator
      * @return true if operation successfully, otherwise false
      */
	bool sendData();

    /**
      * start receive data from FMOD simulator
      * @return true if operation successfully, otherwise false
      */
	bool receiveData();

private:
	boost::asio::ip::tcp::socket socket_;
	FMOD_MsgQueue msgSendQueue;
	FMOD_MsgQueue msgReceiveQueue;
	std::string messageSnd;
	boost::array<char, 1> ReceivedBuf;
};

typedef boost::shared_ptr<FMOD_Client> FmodClientPtr;

}

} /* namespace sim_mob */
#endif /* TCPSESSION_HPP_ */
