//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "ConnectionHandler.hpp"

#include <sstream>

#include "entities/commsim/broker/Broker.hpp"
#include "entities/commsim/client/ClientHandler.hpp"
#include "entities/commsim/serialization/CommsimSerializer.hpp"

using namespace sim_mob;

sim_mob::ConnectionHandler::ConnectionHandler(session_ptr session, BrokerBase& broker)
	: broker(broker), session(session), valid(true), isAsyncWrite(false),
	  isAsyncRead(false)
{
	//Set the token to the pointer address of this ConnectionHandler.
	std::stringstream tk;
	tk <<this;
	token = tk.str();
}


void sim_mob::ConnectionHandler::forwardReadyMessage(ClientHandler& newClient)
{
	//Create it.
	OngoingSerialization ongoing;
	CommsimSerializer::serialize_begin(ongoing, boost::lexical_cast<std::string>(newClient.agent->getId()));
	CommsimSerializer::addGeneric(ongoing, CommsimSerializer::makeReady());

	BundleHeader hRes;
	std::string msg;
	CommsimSerializer::serialize_end(ongoing, hRes, msg);

	//Send it through normal channels.
	forwardMessage(hRes, msg);
}

void sim_mob::ConnectionHandler::forwardMessage(const BundleHeader& head, const std::string& str)
{
	//Send or pend, depending on whether we are in the middle of an existing call to write() or not.
	boost::unique_lock<boost::mutex> lock(async_write_mutex);
	if (isAsyncWrite) {
		pendingMsg.push_front(std::make_pair(head, str));
	} else {
		isAsyncWrite = true;
		sendMessage(head, str);
	}
}


void sim_mob::ConnectionHandler::sendMessage(const BundleHeader& head, const std::string& msg)
{
	outgoingMessage = msg;
	outgoingHeader = head;
	session->async_write(outgoingHeader, outgoingMessage, this);
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
		sendMessage(outgoingHeader, outgoingMessage);
		return;
	}

	//At this point we can schedule another async_write. If the pending list is non-empty, just pull from that.
	{
	boost::unique_lock<boost::mutex> lock(async_write_mutex);
	if (!pendingMsg.empty()) {
		sendMessage(pendingMsg.back().first, pendingMsg.back().second);
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


bool sim_mob::ConnectionHandler::isValid() const
{
	return valid && session->isOpen();
}

std::string sim_mob::ConnectionHandler::getToken() const
{
	return token;
}

void ConnectionHandler::invalidate()
{
	valid = false;
}

std::string sim_mob::ConnectionHandler::getClientType() const
{
	return clientType;
}

void sim_mob::ConnectionHandler::setClientType(const std::string& newCType)
{
	if (!this->clientType.empty()) {
		throw std::runtime_error("Cannot change the client type once it has been set.");
	}
	this->clientType = newCType;
}
