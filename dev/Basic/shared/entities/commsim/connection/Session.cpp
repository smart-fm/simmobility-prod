//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Session.hpp"

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


boost::asio::ip::tcp::socket& sim_mob::Session::socket()
{
  return socket_;
}

bool sim_mob::Session::makeWriteBuffer(std::string &input, std::vector<boost::asio::const_buffer> &output, boost::system::error_code &ec)
{
	// Format the header.
	std::ostringstream header_stream;
	header_stream << std::setw(header_length) << std::hex << input.size();
	if (!header_stream || header_stream.str().size() != header_length) {
		// Something went wrong, inform the caller.
		ec = boost::system::error_code(boost::asio::error::invalid_argument);
		return false;
	}
	outbound_header_ = header_stream.str(); //not used

	output.push_back(boost::asio::buffer(outbound_header_));
	output.push_back(boost::asio::buffer(input));
	return true;
}

bool sim_mob::Session::write(std::string &input, boost::system::error_code &ec)
{
	boost::system::error_code ec_;
	std::vector<boost::asio::const_buffer> buffers;
	makeWriteBuffer(input, buffers, ec_);
	ec = ec_;
	if (ec_) {
		return false;
	}

	boost::asio::write(socket_, buffers, ec_);
	ec = ec_;
	if (ec_) {
		return false;
	}
	return true;
}

bool sim_mob::Session::write(std::string &input)
{
	boost::system::error_code ec_;
	std::vector<boost::asio::const_buffer> buffers;
	makeWriteBuffer(input, buffers, ec_);
	if (ec_) {
		return false;
	}

	boost::asio::write(socket_, buffers, ec_);
	if (ec_) {
		return false;
	}
	return true;
}



