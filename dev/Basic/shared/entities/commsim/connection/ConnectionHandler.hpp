//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include <string>
#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "logging/Log.hpp"
#include "entities/commsim/serialization/BundleVersion.hpp"

namespace sim_mob {
class BrokerBase;
class ClientHandler;
class ConnectionHandler;
class ConnectionServer;

/**
 * This class represents all interactions with a single TCP stream. (Note: The "ClientHandler" can be used to
 *   associate a stream+id with a given Client; it just calls back to this class.)
 * This class has well-defined inputs and outputs, making it clear which functions are called by which
 *   other classes, and furthermore which may be multi-threaded.
 * This class is tightly controlled by the ConnectionServer, hence, it shares friendship with that class.
 */
class ConnectionHandler: public boost::enable_shared_from_this<ConnectionHandler> {
public:
	friend class ConnectionServer;

	ConnectionHandler(boost::asio::io_service& io_service, BrokerBase& broker);

	///Retrieve the type of ClientHandlers that can multiplex on this connection. If empty, any ClientHandler is allowed.
	///If non-empty, only matching types can be multiplexed.
	///NOTE: Clients with the same type are interchangeable; any "new_client" request will dispatch to an *arbitrary*
	///      type upon receiving an "id_request". Current types are "android" and "ns-3".
	std::string getSupportedType() const;

	///Set the ClientHandler type supported by this ConnectionHandler. (NOTE: The first "id_response" message sets this, currently.)
	void setSupportedType(const std::string& type);

	///Retrieve this ConnectionHandler's token, which is used to uniquely identify this ConnectionHandler.
	///  This is used to properly match the "id_response" to the "id_request" sent for it.
	///NOTE: Currently, this is just the address of the ConnectionHandler, in string form (e.g., 0xEF0609...).
	std::string getToken() const;

	///Post a message on this connection. (Does not require locking).
	void postMessage(const BundleHeader& head, const std::string& str);

	///Check if this connection is valid and open.
	bool isValid() const;

	///Invalidate the connection (set valid to false).
	void invalidate();

protected:

	///Start listening for (async_receive()) messages by reading the 8-byte header.
	///NOTE: This function is called once by the ConnectionServer, once the async_accept() has resolved;
	///      it is also called again immediately once handle_read_data has resolved.
	void readHeader();


private:
	///Callback triggered by readHeader()'s async read after 8 bytes are available.
	void handle_read_header(const boost::system::error_code& err);

	///Callback triggered by handle_read_header()'s async read after all remaining bytes are available.
	void handle_read_data(unsigned int rem_len, const boost::system::error_code& err);

	///Used internally to publish messages on the io_service's thread. (NOTE: If we lock in multi-threaded mode anyway, we might not need to post this).
	///NOTE: This parameter *must* be by value, due to the way that post() works.
	void writeMessage(std::string msg);

	///Called by writeMessage() or handle_write() to write the message at the front of the writeQueue
	void writeFrontMessage();

	///Called when the current message has been written. Will trigger writing the next message if it is available.
	void handle_write(const boost::system::error_code& err);


protected:
	///The socket this ConnectionHandler is using for I/O.
	///NOTE: This property is accessed by the ConnectionServer, via its friend status.
	boost::asio::ip::tcp::socket socket;

private:
	///Used for callbacks to the Broker (onNewConnection, etc.)
	BrokerBase& broker;

	///What type of ClientHandlers can be multiplexed onto this connection. Empty = not yet defined (any type).
	std::string supportedType;
	mutable boost::mutex supportedTypeLOCK; //NOTE: Probably don't need to lock this, but it's cheap (and worth being extra-careful here).

	///A token used to uniquely identify this ConnectionHandler.
	std::string token;

	///Whether or not this connection is "valid". The Broker can set this flag, although setting it while
	///  data is incoming or outgoing on the socket leads to undefined behavior.
	bool valid;

	///Saved so that we can post() things.
	boost::asio::io_service& io_service;

	///The size of the largest message we can expect to receive.
	enum { MAX_MSG_LENGTH = 30000 };

	///The message we are currently reading; first 8 bytes are the header.
	char readBuffer[MAX_MSG_LENGTH];

	///The list of messages we are currently writing. front() is the message in progress.
	std::list<std::string> writeQueue;
	boost::mutex writeQueueLOCK;
};

}
