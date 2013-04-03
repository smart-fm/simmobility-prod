//#pragma once

#include <boost/asio.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

#include "ASIO_Impl.hpp"

namespace sim_mob {
/*
 * *************************************************************
 * 						ASIO_Impl
 * * ***********************************************************
 */

ASIO_Impl::ASIO_Impl(DataContainer &mainReceiveBuffer_) :
			mainReceiveBuffer(mainReceiveBuffer_), connection_send(
					io_service_send), connection_receive(io_service_receive) {
	}

	bool ASIO_Impl::connect(boost::asio::io_service &io_service_, connection &connection_,
			const std::string& host, const std::string& service,
			ASIOConnectionType cnnType) {
		// Resolve the host name into an IP address.
		boost::asio::ip::tcp::resolver resolver(io_service_);
		boost::asio::ip::tcp::resolver::query query(host, service);
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
				resolver.resolve(query);
		boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

		// Start an asynchronous connect operation.
		if (cnnType == asio_receive) {
			std::cout << "Start Async Connect to NS3[recv]" << std::endl;
			connection_.socket().async_connect(endpoint,
					boost::bind(&ASIO_Impl::handle_connect_receive, this,
							boost::asio::placeholders::error,
							++endpoint_iterator));
		} else if (cnnType == asio_send) {
			std::cout << "Start Async Connect to NS3[send]" << std::endl;
			connection_.socket().async_connect(endpoint,
					boost::bind(&ASIO_Impl::handle_connect_send, this,
							boost::asio::placeholders::error,
							++endpoint_iterator));
		}
	}

	/// Handle completion of a connect operation.
	void ASIO_Impl::handle_connect_send(const boost::system::error_code& e,
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
		if (!e) {

		} else if (endpoint_iterator
				!= boost::asio::ip::tcp::resolver::iterator()) {
			std::cout << "TRYING THE NEXT END POINT\n";
			// Try the next endpoint.
			connection_send.socket().close();
			boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
			connection_send.socket().async_connect(endpoint,
					boost::bind(&ASIO_Impl::handle_connect_send, this,
							boost::asio::placeholders::error,
							++endpoint_iterator));
		} else {
			// An error occurred. Log it and return. Since we are not starting a new
			// operation the io_service will run out of work to do and the client will
			// exit.
			std::cout << "handle_connect oops\n";
			std::cerr << e.message() << std::endl;
		}
	}
	/// Handle completion of a connect operation.
	void ASIO_Impl::handle_connect_receive(const boost::system::error_code& e,
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
		if (!e) {
			std::cout << "Starting Async Read from NS3" << std::endl;
			connection_receive.async_read(temporaryReceiveBuffer,
					boost::bind(&ASIO_Impl::handle_read_receive, this,
							boost::asio::placeholders::error));
		} else if (endpoint_iterator
				!= boost::asio::ip::tcp::resolver::iterator()) {
			std::cout << "TRYING THE NEXT END POINT\n";
			// Try the next endpoint.
			connection_receive.socket().close();
			boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
			connection_receive.socket().async_connect(endpoint,
					boost::bind(&ASIO_Impl::handle_connect_receive, this,
							boost::asio::placeholders::error,
							++endpoint_iterator));
		} else {
			// An error occurred. Log it and return. Since we are not starting a new
			// operation the io_service will run out of work to do and the client will
			// exit.
			std::cout << "handle_connect oops\n";
			std::cerr << e.message() << std::endl;
		}
	}
	//todo: make this an abstract virtual in the base class
	bool ASIO_Impl::init() {
		//establish two connections to NS3. one for send and one for receive
		connect(io_service_send, connection_send, host_send, port_send,
				asio_send);
		connect(io_service_receive, connection_receive, host_receive,
				port_receive, asio_receive);
		//fire of the thread used for receiving data from NS3
		//todo where are you planning to join this?
		thread_receive = boost::thread(&ASIO_Impl::thread_receive_function,
				this);
	}
	bool ASIO_Impl::thread_receive_function() {
		io_service_receive.run();
	}
	/// Handle completion of a read operation.
	void ASIO_Impl::handle_read_receive(const boost::system::error_code& e) {
		if (!e) {
			mainReceiveBuffer.add(temporaryReceiveBuffer);
			temporaryReceiveBuffer.reset();
		} else {
			// An error occurred.
			std::cout << "handle_read oops\n";
			std::cerr << e.message() << std::endl;
		}

		// Since we are not starting a new operation the io_service will run out of
		// work to do and the client will exit.
	}


}
