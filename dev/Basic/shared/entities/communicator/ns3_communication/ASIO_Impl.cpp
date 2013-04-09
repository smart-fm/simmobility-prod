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
#include<stdio.h>

#include "ASIO_Impl.hpp"

namespace sim_mob {
/*
 * *************************************************************
 * 						ASIO_Impl
 * * ***********************************************************
 */

ASIO_Impl::ASIO_Impl(DataContainer &mainReceiveBuffer_) :
				mainReceiveBuffer(mainReceiveBuffer_),
				connection_send(io_service_send),
				connection_receive(io_service_receive),
				host_send("localhost"),port_send("2012"),
				host_receive("localhost"),port_receive("2013")
{
	sendConnectionEstablished = false;
	init();
}

//todo: make this an abstract virtual in the base class
bool ASIO_Impl::init() {
	//establish two connections to NS3. one for send and one for receive
	//note. for now establish only one connection for receive.
	//and establish a connection for send at each tick when sending the buffer
	//I know it is preferred to have a single connection to send data for all ticks as and when required
	//but there are issues to that. until I get to solve them, we stick to one send connection per tick.
	//todo solve the above mentioned issue.
	connect(io_service_send, connection_send, host_send, port_send,asio_send);
//		std::cout << " ASIO's SendBuffer address " << &mainReceiveBuffer << std::endl;
	connect(io_service_receive, connection_receive, host_receive,port_receive, asio_receive);

	//fire of the thread used for receiving data from NS3
	//todo where are you planning to join this?
	thread_receive_asio = boost::thread(&ASIO_Impl::thread_receive_asio_function,this);
	thread_send_asio = boost::thread(&ASIO_Impl::thread_send_asio_function,this);
}

bool ASIO_Impl::thread_receive_asio_function() {
	io_service_receive.run();
}
bool ASIO_Impl::thread_send_asio_function() {
	io_service_send.run();
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
			std::cout << "ASIO_Impl::connect=>Start Async Connect to NS3[recv]" << std::endl;
			connection_.socket().async_connect(endpoint,boost::bind(&ASIO_Impl::handle_connect_receive, this,boost::asio::placeholders::error,++endpoint_iterator));
		} else if (cnnType == asio_send) {
			std::cout << "ASIO_Impl::connect=>Start Async Connect to NS3[send]" << std::endl;
			connection_.socket().async_connect(endpoint,boost::bind(&ASIO_Impl::handle_connect_send, this,boost::asio::placeholders::error,++endpoint_iterator));
		}
	}

	/// Handle completion of a connect operation.
	void ASIO_Impl::handle_connect_send(const boost::system::error_code& e, boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {

		if (!e) {
			std::cout << "ASIO_Impl::handle_connect_send=>handle_connect_send Request for connection accepted"  << std::endl;
			sendConnectionEstablished = true;
//			//connect (for send) makes a loop that halt on a request to coonect.
			//So be careful the other side doesnt accept connections in a loop in the same way :)
			//feedback: this was a super stupid idea. upon retry to establish the connection, the peer complains, and complains and complains.... try fake read
//				connect(io_service_send, connection_send, host_send, port_send,asio_send);
			//ok, try fake read:
			connection_send.async_read(fakeReceiveBuffer, boost::bind(&ASIO_Impl::handle_read_fake, this,boost::asio::placeholders::error));

		} else {
			sendConnectionEstablished = false;
			std::cerr << "ASIO_Impl::handle_connect_send error =>" << e.message() << " try to connect again" << std::endl;
			connect(io_service_send, connection_send, host_send, port_send,asio_send);
		}
	}
	void ASIO_Impl::handle_send(const boost::system::error_code& e/*, DataContainer &value*/)
	{
	}
	void ASIO_Impl::handle_read_fake(const boost::system::error_code& e/*, DataContainer &value*/)
	{
		if(e)
		{
			std::cout << "ASIO_Impl::handle_read_fake=> "<< e.message() << std::endl;
		}
		connection_send.async_read(fakeReceiveBuffer, boost::bind(&ASIO_Impl::handle_read_fake, this,boost::asio::placeholders::error));
	}
	/// Handle completion of a connect operation.
	void ASIO_Impl::handle_connect_receive(const boost::system::error_code& e,boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
		if (!e) {

			std::cout << "ASIO_Impl::handle_connect_receive=>handle_connect_receive Request for connection accepted, as.read started"  << std::endl;
			connection_receive.async_read(temporaryReceiveBuffer,
					boost::bind(&ASIO_Impl::handle_read_receive, this,
							boost::asio::placeholders::error));
		} else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator()) {
			std::cout << "TRYING THE NEXT END POINT" << std::endl;
		} else {
			std::cerr << "ASIO_Impl::handle_connect_receive error =>" << e.message() << std::endl;
			connect(io_service_receive, connection_receive, host_receive,port_receive, asio_receive);
		}
	}

	/// Handle completion of a read operation.
	void ASIO_Impl::handle_read_receive(const boost::system::error_code& e) {
		if (!e) {
			std::cout << "inside ASIO_Impl::handle_read_receive()" << std::endl;
			mainReceiveBuffer.add(temporaryReceiveBuffer);
			temporaryReceiveBuffer.reset();
			connection_receive.async_read(temporaryReceiveBuffer,
					boost::bind(&ASIO_Impl::handle_read_receive, this,
							boost::asio::placeholders::error));
		} else {
			std::cerr << "ASIO_Impl::handle_read_receive error => "  << e.message() << std::endl;
		}
	}


	//argument not used
	commResult ASIO_Impl::send(DataContainer *value)
	{
//		if(value != &mainReceiveBuffer)
//		{
//			std::ostringstream out("");
//			out << "ASIO::send=>source/ destination mismatch in sending buffer address[" << &mainReceiveBuffer << ":" << value << "]";
//			throw std::runtime_error(out.str());
//		}
		std::cout << "inside ASIO_Impl::send=> starting async_write" << std::endl;
		connection_send.async_write(*value, boost::bind(&ASIO_Impl::handle_send, this,boost::asio::placeholders::error/*, value*/));

	}
	//argument not used
	commResult ASIO_Impl::receive(DataContainer* value){
		//and reference to communicator's receive buffer (here called mainReceiveBuffer)
		//which had passed through ASIO_Impl(()'s constructor,had been already being updated
		//by the a thread handling the receive operasions:)\
		//so just call it with any argument but data will be given at its correct place

		//still for the sake of simplicity and compatibility we will supply the same buffer again:
		value = &mainReceiveBuffer;//ok?
	}



}
