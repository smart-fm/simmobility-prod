//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*#include "Session.hpp"

#include "entities/commsim/connection/ConnectionHandler.hpp"

using namespace sim_mob;

/// Constructor.
sim_mob::Session::Session(boost::asio::io_service &io_service)
  : socket_(io_service)
{
}

sim_mob::Session::~Session()
{
	Print() << "Deleting Session [" << this << "]" << std::endl;
}


boost::asio::ip::tcp::socket& sim_mob::Session::getSocket()
{
	return socket_;
}

bool sim_mob::Session::isOpen() const
{
	return socket_.is_open();
}

std::string sim_mob::Session::MakeWriteBuffer(const BundleHeader& header)
{
	//Format the header.
	std::string res = BundleParser::make_bundle_header(header);
	if (res.empty()) {
		throw std::runtime_error("MakeWriteBuffer: unexpected.");
	}
	return res;
}

bool sim_mob::Session::write(const BundleHeader& header, const std::string &input, boost::system::error_code &ec)
{
	std::vector<boost::asio::const_buffer> buffers;
	outbound_header_ = MakeWriteBuffer(header);
	buffers.push_back(boost::asio::buffer(outbound_header_));
	buffers.push_back(boost::asio::buffer(input));

	boost::asio::write(socket_, buffers, ec);
	return !ec;
}


void sim_mob::Session::async_write(const BundleHeader& header, const std::string &data, ConnectionHandler* handler)
{
	// Format the header.
	outbound_header_ = MakeWriteBuffer(header);

	std::vector<boost::asio::const_buffer> buffers;
	buffers.push_back(boost::asio::buffer(outbound_header_));
	buffers.push_back(boost::asio::buffer(data));
	boost::asio::async_write(socket_, buffers,
		boost::bind(&ConnectionHandler::messageSentHandle, handler, boost::asio::placeholders::error)
	);
}


void sim_mob::Session::async_read(BundleHeader& header, std::string &input, ConnectionHandler* handler)
{
	input.clear();
	inbound_data_.clear();

	// Issue a read operation to read exactly the number of bytes in a header.
	void (Session::*f)(const boost::system::error_code&, BundleHeader&, std::string &, ConnectionHandler*) = &Session::handle_read_header;
	boost::asio::async_read(socket_, boost::asio::buffer(inbound_header_),boost::bind(f,shared_from_this(), boost::asio::placeholders::error, boost::ref(header), boost::ref(input), handler));
}


void sim_mob::Session::handle_read_header(const boost::system::error_code& e, BundleHeader& header, std::string& input, ConnectionHandler* handler)
{
	if (e) {
		handler->messageReceivedHandle(e);
	} else {
		// Determine the length of the serialized data.
		header = BundleParser::read_bundle_header(std::string(inbound_header_, header_length));
		if (header.remLen == 0) {
			// Header doesn't seem to be valid. Inform the caller.
			boost::system::error_code error(boost::asio::error::invalid_argument);
			handler->messageReceivedHandle(error);
			return;
		}

		// Start an asynchronous call to receive the data.
		inbound_data_.resize(header.remLen);
		void (Session::*f)(const boost::system::error_code&, std::string&, ConnectionHandler*) = &Session::handle_read_data;
		boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
			boost::bind(f, shared_from_this(), boost::asio::placeholders::error, boost::ref(input), handler)
		);
	}
}

void sim_mob::Session::handle_read_data(const boost::system::error_code& e, std::string &input, ConnectionHandler* handler) {
	if(!e) {
		std::string archive_data(&inbound_data_[0], inbound_data_.size());
		input = archive_data;
	}
	handler->messageReceivedHandle(e);
}

*/
