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

namespace sim_mob {
class BrokerBase;
class ClientHandler;
class CloudHandler;
class ConnectionServer;

/**
 * A variant of ConnectionHandler that is geared for cloud connections (which vary considerably more than
 *  Android connections).
 */
class CloudHandler: public boost::enable_shared_from_this<CloudHandler> {
public:
	friend class ConnectionServer;

	CloudHandler(boost::asio::io_service& io_service, BrokerBase& broker, const std::string& host, int port);

	boost::asio::ip::tcp::resolver::iterator getResolvedIterator() const;

	//Start reading
	void readLine();

protected:
	///The socket this ConnectionHandler is using for I/O.
	///NOTE: This field is accessed by the ConnectionServer via its friend status.
	boost::asio::ip::tcp::socket socket;

private:
	///Used for callbacks to the Broker.
	BrokerBase& broker;

	///Our tcp endpoint, resolved into an iterator.
	boost::asio::ip::tcp::resolver::iterator resolvedIt;
};

}
