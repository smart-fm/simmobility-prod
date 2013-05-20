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
#include "entities/communicator/Communication_Base.hpp"
#include "entities/Agent.hpp"
#include <boost/thread.hpp>
#include "connection.hpp"

namespace sim_mob {

class BoostSerialized_Client_ASIO: public Communication<DataContainer&, commResult> {
	boost::asio::io_service io_service_send, io_service_receive ,io_service_;
//	boost::asio::ip::tcp::socket socket_send, socket_receive;
	connection connection_;

	//todo: make them constant reference
	std::string port_;
	std::string host_;

	DataContainer temporaryReceiveBuffer;
	DataContainer *temporarySendBuffer;
	DataContainer fakeReceiveBuffer; //to be used by connection_send to keep the socket alive
//	DataContainer &mainSendBuffer;
	DataContainer &mainReceiveBuffer_;
	boost::thread thread_send_asio;
	boost::thread thread_receive_asio;
	boost::thread thread_asio;
	bool sendConnectionEstablished;

	enum ASIOConnectionType {
		asio_receive, asio_send
	};
public:
	BoostSerialized_Client_ASIO( std::string host,std::string port,DataContainer &mainReceiveBuffer_);
	bool connect();

	/// Handle completion of a connect operation.
	void handle_connect(const boost::system::error_code& e);
	void handle_read(const boost::system::error_code& error);
	void handle_send(const boost::system::error_code& e, DataContainer &value);
	bool init();
	bool thread_asio_function();
	void close();
	void do_close();

	//the polymorphism thing

	commResult send(DataContainer &value);
	commResult receive(DataContainer& value);
	~BoostSerialized_Client_ASIO();
};

}
