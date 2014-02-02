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


boost::asio::ip::tcp::socket& sim_mob::Session::getSocket()
{
	return socket_;
}

bool sim_mob::Session::isOpen() const
{
	return socket_.is_open();
}

std::string sim_mob::Session::MakeWriteBuffer(const std::string &input)
{
	// Format the header.
	std::ostringstream header_stream;
	header_stream <<std::setw(header_length) <<std::hex <<input.size();
	if (!header_stream || header_stream.str().size() != header_length) {
		throw std::runtime_error("MakeWriteBuffer: unexpected.");
	}

	//Done.
	return header_stream.str();
}

bool sim_mob::Session::write(const std::string &input, boost::system::error_code &ec)
{
	std::vector<boost::asio::const_buffer> buffers;
	outbound_header_ = MakeWriteBuffer(input);
	buffers.push_back(boost::asio::buffer(outbound_header_));
	buffers.push_back(boost::asio::buffer(input));

	boost::asio::write(socket_, buffers, ec);
	return !ec;
}

