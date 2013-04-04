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
//todo reduce the includes
#include "../Communication_Base.hpp"
#include "entities/Agent.hpp"
#include <boost/thread.hpp>
#include "../serialized_asio/connection.hpp"

namespace sim_mob {

class ASIO_Impl: public Communication<DataContainer&, commResult> {
	boost::asio::io_service io_service_send, io_service_receive;
//	boost::asio::ip::tcp::socket socket_send, socket_receive;
	connection connection_send, connection_receive;

	//todo: make them constant reference
	std::string host_send;
	std::string port_send;
	std::string host_receive;
	std::string port_receive;

	DataContainer temporaryReceiveBuffer;
	DataContainer *temporarySendBuffer;

	DataContainer &mainReceiveBuffer;
	boost::thread thread_receive;

	bool sendConnectionEstablished;

	enum ASIOConnectionType {
		asio_receive, asio_send
	};
public:
	ASIO_Impl(DataContainer &mainReceiveBuffer_);
	bool connect(boost::asio::io_service &io_service_, connection &connection_,
			const std::string& host, const std::string& service,
			ASIOConnectionType cnnType);

	/// Handle completion of a connect operation.
	void handle_connect_send(const boost::system::error_code& e,
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
	/// Handle completion of a connect operation.
	void handle_connect_receive(const boost::system::error_code& e,
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
	void handle_send(const boost::system::error_code& e/*, DataContainer &value*/);
	bool init();
	bool thread_receive_function();
	/// Handle completion of a read operation.
	void handle_read_receive(const boost::system::error_code& e);

	//the polymorphism thing

	commResult send(DataContainer &value);
	commResult receive(DataContainer& value);

};

}
