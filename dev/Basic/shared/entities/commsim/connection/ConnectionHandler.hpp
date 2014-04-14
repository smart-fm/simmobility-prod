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
#include "entities/commsim/connection/Session.hpp"

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

	//NOTE: Passing "callback" by value and then saving it by reference is a bad idea!
	//      For now I've made both work by value; you may need to modify this. ~Seth
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



	//Send a "READY" message to the given client.
	//void forwardReadyMessage(ClientHandler& newClient);

	//Send a message on behalf of the given client.
	//void forwardMessage(const BundleHeader& head, const std::string& str);

	bool isValid() const;
	void invalidate();

	///Callback used by Session to indicate a message has been sent.
	//void messageSentHandle(const boost::system::error_code &e);

	///Callback used by Session to indicate a message has been received.
	//void messageReceivedHandle(const boost::system::error_code& e);

private:
	//Helper: send a message, read a message.
	//void sendMessage(const BundleHeader& head, const std::string& msg);
	//void readMessage();

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

	///A token used to uniquely identify this ConnectionHandler.
	std::string token;

	///Saved so that we can post() things.
	boost::asio::io_service& io_service;

	///The size of the largest message we can expect to receive.
	enum { MAX_MSG_LENGTH = 30000 };

	///The message we are currently reading; first 8 bytes are the header.
	char readBuffer[MAX_MSG_LENGTH];

	///The list of messages we are currently writing. front() is the message in progress.
	std::list<std::string> writeQueue;



	//session_ptr session;
	//boost::function<void(boost::shared_ptr<ConnectionHandler>, std::string)> messageReceiveCallback;

	bool valid;

	//The current incoming/outgoing messages/headers. Boost wants these passed by reference.
/*	BundleHeader outgoingHeader;
	std::string outgoingMessage;
	std::string incomingMessage;
	BundleHeader incomingHeader;

	//The following variables relate to the actual sending of data on this ConnectionHandler:
	//These mutexes lock them, as they can arrive in a non-thread-safe manner.
	boost::mutex async_write_mutex;
	boost::mutex async_read_mutex;

	//Lock async_write. If this flag is true, we can't call async_write, so outgoing messages are pended to an array.
	bool isAsyncWrite;
	std::list< std::pair<BundleHeader, std::string> > pendingMsg; //Messages pending to be sent out.

	//Lock async_read. If this flag is true, we can't call async_read until the current operation completes.
	bool isAsyncRead;
	//long pendingReads; //How many "read" operations are pending.

*/
};

}
