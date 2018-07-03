//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include <string>
#include <list>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace sim_mob {
class BrokerBase;
class ClientHandler;
class CloudHandler;
class ConnectionServer;

/**
 * A variant of ConnectionHandler that is geared for cloud connections (which vary considerably more than
 *  Android connections).
 * Note that unlike the ConnectionHandler, the CloudHandler is mostly client-driven. In other words, it operates
 *  on a transaction of Broker->write(lines)->read(lines)->Broker.
 */
class CloudHandler: public boost::enable_shared_from_this<CloudHandler> {
protected:
	friend class ConnectionServer;

	/**
	 * Create a new CloudHandler, which will multiplex on the given io_service, call back to the given broker,
	 *   and connect to the given host:port. NOTE: In practice, you will usually call ConnectionServer::connectToCloud()
	 *   to safely create a new CloudHandler.
	 */
	CloudHandler(boost::asio::io_service& io_service, BrokerBase& broker, const std::string& host, int port);

	/**
	 * Retrieve the iterator to the host:port that this CloudHandler was created with. This can then be used
	 *   to begin or continue asynchronous i/o. NOTE: This function is typically only used by the ConnectionServer.
	 */
	boost::asio::ip::tcp::resolver::iterator getResolvedIterator() const;

public:
	/**
	 * Write several lines of data to the cloud server and then read a response. This is a synchronous operation.
	 * \param outgoing The lines to be sent to the cloud.
	 * \returns The lines that were received from the server after the outgoing array was written.
	 */
	std::vector<std::string> writeLinesReadLines(const std::vector<std::string>& outgoing);

protected:
	///The socket this ConnectionHandler is using for I/O.
	///NOTE: This field is accessed by the ConnectionServer via its friend status.
	boost::asio::ip::tcp::socket socket;

	///We only allow one write->read loop at a time, which we lock with a mutex+condition.
	bool isWriteRead;
	boost::mutex mutex_is_write_read;
	boost::condition_variable COND_VAR_IS_WRITE_READ;

private:
	///Used for callbacks to the Broker.
	BrokerBase& broker;

	///Our tcp endpoint, resolved into an iterator.
	boost::asio::ip::tcp::resolver::iterator resolvedIt;
};

}
