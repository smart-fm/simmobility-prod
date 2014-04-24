//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "CloudHandler.hpp"

#include <stdexcept>
#include <boost/lexical_cast.hpp>

using namespace sim_mob;

sim_mob::CloudHandler::CloudHandler(boost::asio::io_service& io_service, BrokerBase& broker, const std::string& host, int port)
	: broker(broker), socket(io_service)
{
	//Resolve the remote address.
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query(host, boost::lexical_cast<std::string>(port));
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
}

boost::asio::ip::tcp::resolver::iterator sim_mob::CloudHandler::getResolvedIterator() const
{
	return resolvedIt;
}


void sim_mob::CloudHandler::readLine()
{
	throw std::runtime_error("TODO: CloudHandler::readLine()");
}
