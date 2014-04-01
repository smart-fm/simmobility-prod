//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "ConnectionHandler.hpp"

#include <sstream>

#include "entities/commsim/Broker.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "entities/commsim/serialization/CommsimSerializer.hpp"

using namespace sim_mob;

sim_mob::ConnectionHandler::ConnectionHandler(session_ptr session, BrokerBase& broker)
	: broker(broker), clientType(comm::UNKNOWN_CLIENT), session(session), valid(true), isAsyncWrite(false),
	  isAsyncRead(false)
{
	//Set the token to the pointer address of this ConnectionHandler.
	std::stringstream tk;
	tk <<this;
	token = tk.str();
}


void sim_mob::ConnectionHandler::forwardReadyMessage(ClientHandler& newClient)
{
	//Send it through normal channels.
	forwardMessage(CommsimSerializer::makeReady());
}

void sim_mob::ConnectionHandler::forwardMessage(std::string str)
{
	//Send or pend, depending on whether we are in the middle of an existing call to write() or not.
	boost::unique_lock<boost::mutex> lock(async_write_mutex);
	if (isAsyncWrite) {
		pendingMsg.push_front(str);
	} else {
		isAsyncWrite = true;
		sendMessage(str);
	}
}


void sim_mob::ConnectionHandler::sendMessage(const std::string& msg)
{
	outgoingMessage = msg;
	session->async_write(outgoingMessage, this);
}

void sim_mob::ConnectionHandler::readMessage()
{
	session->async_read(incomingHeader, incomingMessage, this);
}


void sim_mob::ConnectionHandler::messageSentHandle(const boost::system::error_code &e)
{
	//If there's an error, we can just re-send it (we are still protected by isAsyncWrite).
	if(e) {
		Warn() << "Connection Not Ready[" << e.message() << "] Trying Again" << std::endl;
		sendMessage(outgoingMessage);
		return;
	}

	//At this point we can schedule another async_write. If the pending list is non-empty, just pull from that.
	{
	boost::unique_lock<boost::mutex> lock(async_write_mutex);
	if (!pendingMsg.empty()) {
		sendMessage(pendingMsg.back());
		pendingMsg.pop_back();
	} else {
		//We're done writing; the next write will have to be triggered by a call to forwardMessage().
		isAsyncWrite = false;
	}
	} //async_write_mutex unlocks

	//Typically, we would listen for an incoming message here. We have to make sure we are not waiting on a write-lock, however.
	boost::unique_lock<boost::mutex> lock(async_read_mutex);
	if (!isAsyncRead) {
		isAsyncRead = true;
		readMessage();
	}
}

void sim_mob::ConnectionHandler::messageReceivedHandle(const boost::system::error_code& e)
{
	if(e) {
		Warn() <<"Read Fail [" << e.message() << "]" << std::endl;
		return;
	}

	//call the receive handler in the broker
	broker.onMessageReceived(shared_from_this(), incomingHeader, incomingMessage);

	//Always expect a new message.
	boost::unique_lock<boost::mutex> lock(async_read_mutex);
	readMessage();
}

session_ptr& sim_mob::ConnectionHandler::getSession()
{
	return session;
}

bool sim_mob::ConnectionHandler::is_open() const
{
	return session->isOpen();
}

std::string sim_mob::ConnectionHandler::getToken() const
{
	return token;
}

bool sim_mob::ConnectionHandler::isValid() const
{
	return valid;
}

void ConnectionHandler::setValidation(bool value)
{
	valid = value;
}

sim_mob::comm::ClientType sim_mob::ConnectionHandler::getClientType() const
{
	return clientType;
}

void sim_mob::ConnectionHandler::setClientType(sim_mob::comm::ClientType newCType)
{
	if (this->clientType != comm::UNKNOWN_CLIENT) {
		throw std::runtime_error("Cannot change the client type once it has been set.");
	}
	this->clientType = newCType;
}
