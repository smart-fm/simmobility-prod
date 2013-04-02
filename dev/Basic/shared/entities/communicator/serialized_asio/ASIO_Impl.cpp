#pragma once

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

#include "Communication.hpp"
#include "entities/Agent.hpp"
#include <boost/thread.hpp>


namespace sim_mob
{

class ASIO_Impl: public Communication<DataContainer&, commResult>  {
	boost::asio::io_service io_service_send, io_service_receive;
	boost::asio::ip::tcp::socket socket_send, socket_receive;
	connection connection_send, connection_receive;

	const std::string& host_send;
	const std::string& port_send;
	const std::string& host_receive;
	const std::string& port_receive;

	DataContainer temporaryReceiveBuffer;

	DataContainer &mainReceiveBuffer;
	boost::thread thread_receive;

	bool endReceiveThread;
	typedef void (*handle_connect)(const boost::system::error_code& e,
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

	enum ASIOConnectionType
	{
		asio_receive,
		asio_send
	};
public:
	ASIO_Impl(DataContainer &mainReceiveBuffer_) :
			mainReceiveBuffer(mainReceiveBuffer_)
			,connection_send(io_service_send)
			,connection_receive(io_service_receive) {

		endReceiveThread = false;
	}

	bool connect(
			boost::asio::io_service io_service_
			,connection connection_
			,const std::string& host
			,const std::string& service
			,handle_connect handle_connect_
			,ASIOConnectionType cnnType
			) {
		// Resolve the host name into an IP address.
		boost::asio::ip::tcp::resolver resolver(io_service_);
		boost::asio::ip::tcp::resolver::query query(host, service);
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
				resolver.resolve(query);
		boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

		// Start an asynchronous connect operation.
		if(cnnType == asio_receive)
		{
			std::cout << "Start Async Connect to NS3[recv]" << std::endl;
			connection_.socket().async_connect(endpoint,
				boost::bind(&ASIO_Impl::handle_connect_receive, this,
						boost::asio::placeholders::error, ++endpoint_iterator));
		}
		else if(cnnType == asio_send)
		{
			std::cout << "Start Async Connect to NS3[send]" << std::endl;
			connection_.socket().async_connect(endpoint,
				boost::bind(&ASIO_Impl::handle_connect_send, this,
						boost::asio::placeholders::error, ++endpoint_iterator));
		}
	}

	/// Handle completion of a connect operation.
	void handle_connect_send(const boost::system::error_code& e,
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
		if (!e) {

		} else if (endpoint_iterator
				!= boost::asio::ip::tcp::resolver::iterator()) {
			std::cout << "TRYING THE NEXT END POINT\n";
			// Try the next endpoint.
			connection_.socket().close();
			boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
			connection_.socket().async_connect(endpoint,
					boost::bind(&client::handle_connect, this,
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
	void handle_connect_receive(const boost::system::error_code& e,
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
			connection_.socket().close();
			boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
			connection_.socket().async_connect(endpoint,
					boost::bind(&client::handle_connect, this,
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
	bool init() {
		//establish two connections to NS3. one for send and one for receive
		connect(io_service_send,connection_send,host_send,port_send);
		connect(io_service_receive,connection_receive,host_receive,port_receive);
		//fire of the thread used for receiving data from NS3
		thread_receive = boost::thread(&ASIO_Impl::thread_receive,this);
	}
	bool thread_receive()
	{
		io_service_receive.run();
	}
	  /// Handle completion of a read operation.
	  void handle_read_receive(const boost::system::error_code& e)
	  {
	    if (!e)
	    {
	    	mainReceiveBuffer.add(temporaryReceiveBuffer);
	    	temporaryReceiveBuffer.reset();
	    }
	    else
	    {
	      // An error occurred.
	    	std::cout << "handle_read oops\n";
	    	std::cerr << e.message() << std::endl;
	    }

	    // Since we are not starting a new operation the io_service will run out of
	    // work to do and the client will exit.
	  }


};

}
