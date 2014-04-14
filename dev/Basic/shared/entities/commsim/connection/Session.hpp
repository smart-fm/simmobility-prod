//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)
/*
#pragma once

#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include "logging/Log.hpp"
#include "entities/commsim/serialization/BundleVersion.hpp"

namespace sim_mob {

class Session;
class ConnectionHandler;


//Helper: Pointer to a session.
typedef boost::shared_ptr<Session> session_ptr;
*/

/// The session class provides serialization primitives on top of a socket.
/**
 * Each message sent using this class consists of:
 * 8-byte header containing the length of the serialized data in hexadecimal.
 * and then The serialized data.
 */
/*class Session : public boost::enable_shared_from_this<Session>
{
public:
	/// Constructor.
	Session(boost::asio::io_service &io_service);
	~Session();

	///Get the underlying socket. Used by the ConnectionServer to continue looping to the next connection.
	boost::asio::ip::tcp::socket& getSocket();

	///Is this Session's socket open?
	bool isOpen() const;

	//Write data immediately to the underlying socket. WARNING: Almost never do this! Use the async functions.
	bool write(const BundleHeader& header, const std::string &input, boost::system::error_code &ec);

	/// Asynchronously write a data structure to the socket.
	void async_write(const BundleHeader& header, const std::string &data, ConnectionHandler* handler);

	/// Asynchronously read a data structure from the socket.
	void async_read(BundleHeader& header, std::string &input, ConnectionHandler* handler);

private:
	static std::string MakeWriteBuffer(const BundleHeader& header);


	/// Handle a completed read of a message header. The handler is passed using
	/// a tuple since boost::bind seems to have trouble binding a function object
	/// created using boost::bind as a parameter.
	void handle_read_header(const boost::system::error_code& e, BundleHeader& header, std::string &input, ConnectionHandler* handler);


	/// Handle a completed read of message data.
	void handle_read_data(const boost::system::error_code& e,std::string &input, ConnectionHandler* handler);

private:
	/// The underlying socket.
	boost::asio::ip::tcp::socket socket_;

	/// The size of a fixed length header.
	enum { header_length = 8 };

	/// Holds an outbound header.
	std::string outbound_header_;

	/// Holds an inbound header.
	char inbound_header_[header_length];

	/// Holds the inbound data.
	std::vector<char> inbound_data_;
};

}
*/
