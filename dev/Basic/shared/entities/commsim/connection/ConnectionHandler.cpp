//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "ConnectionHandler.hpp"

#include <sstream>

#include "entities/commsim/Broker.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "entities/commsim/serialization/CommsimSerializer.hpp"

using namespace sim_mob;

sim_mob::ConnectionHandler::ConnectionHandler(session_ptr session, boost::function<void(boost::shared_ptr<ConnectionHandler>, std::string)> messageReceiveCallback_/*, sim_mob::comm::ClientType clientType*/)
	: messageReceiveCallback(messageReceiveCallback_), clientType(comm::UNKNOWN_CLIENT), session(session), valid(true), isAsyncWrite(false),
	  isAsyncRead(false)
{
	//Set the token to the pointer address of this ConnectionHandler.
	std::stringstream tk;
	tk <<this;
	token = tk.str();
}


void sim_mob::ConnectionHandler::forwardReadyMessage(ClientHandler& newClient)
{
	//Write the header/message body.
	Json::Value packet_header = JsonParser::createPacketHeader(pckt_header(1, newClient.clientId));
	Json::Value msg = JsonParser::createMessageHeader(msg_header("0","SIMMOBILITY","READY", "SYS"));

	//Put them together into a single "packet".
	Json::Value packet;
	packet["PACKET_HEADER"] = packet_header;
	packet["DATA"].append(msg);//no other data element needed
	std::string readyMessage = Json::FastWriter().write(packet);

	//Send it through normal channels.
	forwardMessage(readyMessage);
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
	session->async_write(outgoingMessage,boost::bind(&ConnectionHandler::messageSentHandle, this, boost::asio::placeholders::error,outgoingMessage));
}

void sim_mob::ConnectionHandler::readMessage()
{
	session->async_read(incomingMessage, boost::bind(&ConnectionHandler::messageReceivedHandle, this, boost::asio::placeholders::error));
}


void sim_mob::ConnectionHandler::messageSentHandle(const boost::system::error_code &e, std::string str)
{
	//If there's an error, we can just re-send it (we are still protected by isAsyncWrite).
	if(e) {
		Warn() << "Connection Not Ready[" << e.message() << "] Trying Again" << std::endl;
		sendMessage(str);
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
	messageReceiveCallback(shared_from_this(),incomingMessage);

	//Keep reading?
	boost::unique_lock<boost::mutex> lock(async_read_mutex);
	//if (pendingReads > 0) {
	readMessage();
	//} else {
	//	isAsyncRead = false;
	//}
}

/*void sim_mob::ConnectionHandler::sendImmediately(const std::string& str)
{
	boost::unique_lock<boost::mutex> lock1(async_write_mutex);
	boost::unique_lock<boost::mutex> lock2(async_read_mutex);
	if (isAsyncRead || isAsyncWrite) {
		throw std::runtime_error("Sending data in immediate mode on an asynchronous socket.");
	}

	boost::system::error_code ec;
	outgoingMessage = str;
	session->write(outgoingMessage, ec);
}*/

session_ptr& sim_mob::ConnectionHandler::getSession()
{
	return session;
}

bool sim_mob::ConnectionHandler::is_open()
{
	return session->isOpen();
}

std::string sim_mob::ConnectionHandler::getToken() const
{
	return token;
}

bool sim_mob::ConnectionHandler::isValid()
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
