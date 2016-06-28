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
	{
	boost::lock_guard<boost::mutex> lock(writeQueueLOCK);
	alreadyWriting = !writeQueue.empty();
	writeQueue.push_back(msg);
	}

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
	bool empty = false;
	{
	boost::lock_guard<boost::mutex> lock(writeQueueLOCK);
	writeQueue.pop_front();
	empty = writeQueue.empty();
	}

	//Is there anything else in the queue to write?
	if (!empty) {
		writeFrontMessage();
	}
}


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
	boost::lock_guard<boost::mutex> lock(supportedTypeLOCK);
	return supportedType;
}

void sim_mob::ConnectionHandler::setSupportedType(const std::string& type)
{
	boost::lock_guard<boost::mutex> lock(supportedTypeLOCK);
	if (!this->supportedType.empty()) {
		Warn() <<"Warning: Changing non-empty connection type (from,to) = (" <<supportedType <<"," <<supportedType <<")";
	}
	this->supportedType = type;
}
