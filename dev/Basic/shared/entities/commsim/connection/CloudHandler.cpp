//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "CloudHandler.hpp"

#include <stdexcept>
#include <boost/lexical_cast.hpp>

using namespace sim_mob;

sim_mob::CloudHandler::CloudHandler(boost::asio::io_service& io_service, BrokerBase& broker, const std::string& host, int port)
	: broker(broker), socket(io_service), isWriteRead(false)
{
	//Resolve the remote address.
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query(host, boost::lexical_cast<std::string>(port));
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
}


std::vector<std::string> sim_mob::CloudHandler::writeLinesReadLines(const std::vector<std::string>& outgoing)
{
	{ //Wait for the write/read lock and claim it.
	boost::unique_lock<boost::mutex> lock(mutex_is_write_read);
	while (!isWriteRead) {
		COND_VAR_IS_WRITE_READ.wait(lock);
	}
	isWriteRead = true;
	}

	//At the moment, we can only handle single-line writes. Multi-line is not much more difficult, but
	// the policy of write/read interleaving has to be decided.
	if (outgoing.size()!=1) {
		throw std::runtime_error("Can't handle multi-line cloud writes at the moment.");
	}

	//Now, write it (synchronously)
	std::string currWrite = outgoing.front();
	if (currWrite[currWrite.size()-1] != '\n') {
		currWrite += "\n";
	}
	boost::asio::write(socket, boost::asio::buffer(currWrite));

	//At this point, we have to read any number of lines back from the server, until a totally empty line ("\n") is encountered.
	std::vector<std::string> res;
	boost::asio::streambuf buff;

	//Keep reading into the same buffer (REQUIRED, since read_until may store additional characters in the buffer for later).
	for (;;) {
		//Read into the back of the array.
		res.push_back("");
		boost::asio::read_until(socket, buff, "\n");
		std::istream is(&buff);
		std::getline(is, res.back());

		//Done?
		if (res.back() == "\n") {
			break;
		}
	}

	{ //Unlock, notify
	boost::unique_lock<boost::mutex> lock(mutex_is_write_read);
	isWriteRead = false;
	}
	COND_VAR_IS_WRITE_READ.notify_all();

	//Done
	return res;
}



boost::asio::ip::tcp::resolver::iterator sim_mob::CloudHandler::getResolvedIterator() const
{
	return resolvedIt;
}

