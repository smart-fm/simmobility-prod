//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "ConnectionHandler.hpp"

#include <sstream>

#include "entities/commsim/broker/Broker.hpp"
#include "entities/commsim/client/ClientHandler.hpp"
#include "entities/commsim/serialization/CommsimSerializer.hpp"

using namespace sim_mob;

sim_mob::ConnectionHandler::ConnectionHandler(boost::asio::io_service& io_service, BrokerBase& broker)
	: broker(broker), socket(io_service), valid(true), io_service(io_service)
{
	//Set the token to the pointer address of this ConnectionHandler.
	std::stringstream tk;
	tk <<this;
	token = tk.str();
}

void sim_mob::ConnectionHandler::readHeader()
{
	boost::asio::async_read(socket, boost::asio::buffer(readBuffer, 8),
		boost::bind(&ConnectionHandler::handle_read_header, this, boost::asio::placeholders::error)
	);
}

void sim_mob::ConnectionHandler::handle_read_header(const boost::system::error_code& err)
{
	//Stop reading on errors.
	if (err) {
		Warn() <<"Error reading from client: " <<err.message() <<"\n";
		return;
	}

	//Decode the remaining length.
	unsigned int rem_len = ((int(readBuffer[4])&0xFF)<<24) | ((int(readBuffer[5])&0xFF)<<16) | ((int(readBuffer[6])&0xFF)<<8) | (int(readBuffer[7])&0xFF);
	if (rem_len==0 || rem_len+8 >= MAX_MSG_LENGTH) {
		Warn() <<"Error: Client message length (" <<rem_len <<") is zero, or exceeds max length: " <<MAX_MSG_LENGTH <<"\n";
		return;
	}

	//Now read the data section (into the same buffer).
	boost::asio::async_read(socket, boost::asio::buffer((readBuffer+8), rem_len),
		boost::bind(&ConnectionHandler::handle_read_data, this, rem_len, boost::asio::placeholders::error)
	);
}


void sim_mob::ConnectionHandler::handle_read_data(unsigned int rem_len, const boost::system::error_code& err)
{
	//Stop reading on errors.
	if (err) {
		Warn() <<"Error reading from client: " <<err.message() <<"\n";
		return;
	}

	//Post it to the Broker (as a string copy).
	broker.onMessageReceived(shared_from_this(), readBuffer, rem_len+8);

	//Now read a new header.
	readHeader();
}


void sim_mob::ConnectionHandler::postMessage(const BundleHeader& head, const std::string& str)
{
	//Create it.
	std::stringstream res;
	res <<BundleParser::make_bundle_header(head);
	res <<str;

	io_service.post(boost::bind(&ConnectionHandler::writeMessage, this, res.str()));
}


void sim_mob::ConnectionHandler::writeMessage(std::string msg)
{
	//Add the message.
	bool alreadyWriting = false;
	alreadyWriting = !writeQueue.empty();
	writeQueue.push_back(msg);

	//"Wake" if this is the first new message in the queue.
	if (!alreadyWriting) {
		writeFrontMessage();
	}
}


void sim_mob::ConnectionHandler::writeFrontMessage()
{
	boost::asio::async_write(socket, boost::asio::buffer(writeQueue.front()),
		boost::bind(&ConnectionHandler::handle_write, this, boost::asio::placeholders::error)
	);
}


void sim_mob::ConnectionHandler::handle_write(const boost::system::error_code& err)
{
	//Stop writing on errors.
	if (err) {
		Warn() <<"Error writing to client: " <<err.message() <<"\n";
		return;
	}

	//Remove this message; it's been written correctly.
	writeQueue.pop_front();
	bool empty = writeQueue.empty();

	//Is there anything else in the queue to write?
	if (!empty) {
		writeFrontMessage();
	}
}






/*void sim_mob::ConnectionHandler::forwardReadyMessage(ClientHandler& newClient)
{
	//Create it.
	OngoingSerialization ongoing;
	CommsimSerializer::serialize_begin(ongoing, boost::lexical_cast<std::string>(newClient.clientId));
	CommsimSerializer::addGeneric(ongoing, CommsimSerializer::makeIdAck());

	BundleHeader hRes;
	std::string msg;
	CommsimSerializer::serialize_end(ongoing, hRes, msg);

	//Send it through normal channels.
	forwardMessage(hRes, msg);
}*/

/*void sim_mob::ConnectionHandler::forwardMessage(const BundleHeader& head, const std::string& str)
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
}*/


bool sim_mob::ConnectionHandler::isValid() const
{
	return valid && socket.is_open();
}

std::string sim_mob::ConnectionHandler::getToken() const
{
	return token;
}

void ConnectionHandler::invalidate()
{
	valid = false;
}

std::string sim_mob::ConnectionHandler::getSupportedType() const
{
	return supportedType;
}

void sim_mob::ConnectionHandler::setSupportedType(const std::string& type)
{
	if (!this->supportedType.empty()) {
		Warn() <<"Warning: Changing non-empty connection type (from,to) = (" <<supportedType <<"," <<supportedType <<")";
	}
	this->supportedType = type;
}
