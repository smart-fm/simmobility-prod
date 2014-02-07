//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ConnectionHandler.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#pragma once

#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "logging/Log.hpp"
#include "entities/commsim/connection/Session.hpp"
#include "entities/commsim/client/ClientType.hpp"

namespace sim_mob {
class ClientHandler;
class ConnectionHandler;
class ConnectionHandler: public boost::enable_shared_from_this<ConnectionHandler> {
public:
	//NOTE: Passing "callback" by value and then saving it by reference is a bad idea!
	//      For now I've made both work by value; you may need to modify this. ~Seth
	ConnectionHandler(session_ptr session , boost::function<void(boost::shared_ptr<ConnectionHandler>, std::string)> messageReceiveCallback_/*, sim_mob::comm::ClientType clientType = sim_mob::comm::UNKNOWN_CLIENT*/);

	//Send a "READY" message to the given client.
	void forwardReadyMessage(ClientHandler& newClient);

	//Send a message on behalf of the given client.
	void forwardMessage(std::string str);

	void messageSentHandle(const boost::system::error_code &e, std::string str);
	void messageReceivedHandle(const boost::system::error_code& e);

	session_ptr& getSession();
	bool is_open();
	bool isValid();
	void setValidation(bool);

	//NOTE: The first WHOAMI message that arrives at this ConnectionHandler sets the expected ClientType.
	sim_mob::comm::ClientType getClientType() const;
	void setClientType(sim_mob::comm::ClientType newCType);

	//Send a synchronous message. NOTE: Be careful with this! The ConnectionHandler is designed for
	// asynchronous sending.
	void sendImmediately(const std::string& str);

	//A token is used to uniquely identify ConnectionHandlers.
	std::string getToken() const;

private:
	//Helper: send a message, read a message.
	void sendMessage(const std::string& msg);
	void readMessage();

	//What type of clients (Android, NS3) can this ConnectionHandler manage? Starts off as Unknown.
	sim_mob::comm::ClientType clientType;

	session_ptr session;
	boost::function<void(boost::shared_ptr<ConnectionHandler>, std::string)> messageReceiveCallback;
	bool valid;

	//The current incoming/outgoing messages. Boost wants these passed by reference.
	std::string outgoingMessage;
	std::string incomingMessage;

	//The following variables relate to the actual sending of data on this ConnectionHandler:
	//These mutexes lock them, as they can arrive in a non-thread-safe manner.
	boost::mutex async_write_mutex;
	boost::mutex async_read_mutex;

	//Lock async_write. If this flag is true, we can't call async_write, so outgoing messages are pended to an array.
	bool isAsyncWrite;
	std::list<std::string> pendingMsg; //Messages pending to be sent out.

	//Lock async_read. If this flag is true, we can't call async_read until the current operation completes.
	bool isAsyncRead;
	//long pendingReads; //How many "read" operations are pending.

	std::string token;
};

}
