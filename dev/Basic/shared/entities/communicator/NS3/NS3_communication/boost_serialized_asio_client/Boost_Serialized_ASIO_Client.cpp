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

#include "Boost_Serialized_ASIO_Client.hpp"
/*
 * *************************************************************
 * 						BoostSerialized_Client_ASIO
 * * ***********************************************************
 */
namespace sim_mob {
BoostSerialized_Client_ASIO::BoostSerialized_Client_ASIO(std::string host,std::string port,DataContainer &mainReceiveBuffer):
		connection_(io_service_),host_(host),port_(port),mainReceiveBuffer_(mainReceiveBuffer)
{
	init();
}

bool BoostSerialized_Client_ASIO::init()
{
	connect();
}

bool BoostSerialized_Client_ASIO::connect()
{

	// Resolve the host name into an IP address.
	boost::asio::ip::tcp::resolver resolver(io_service_);
	boost::asio::ip::tcp::resolver::query query(host_, port_);
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
			resolver.resolve(query);
	boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

	std::cout << "Connecting" << std::endl;
	boost::asio::async_connect(connection_.socket(), endpoint_iterator, boost::bind(&BoostSerialized_Client_ASIO::handle_connect, this,boost::asio::placeholders::error));
	boost::thread thread_asio = boost::thread(&BoostSerialized_Client_ASIO::thread_asio_function,this);
}

bool BoostSerialized_Client_ASIO::thread_asio_function() {
	io_service_.run();
}

void BoostSerialized_Client_ASIO::close()
{
  io_service_.post(boost::bind(&BoostSerialized_Client_ASIO::do_close, this));
}

void BoostSerialized_Client_ASIO::handle_connect(const boost::system::error_code& error)
{
  if (!error)
  {
//	  boost::unique_lock< boost::shared_mutex > lock(*mainReceiveBuffer_.Owner_Mutex);
	  std::cout << "Connection Established" << std::endl;
	  std::cout << "Reading" << std::endl;
	  connection_.async_read(temporaryReceiveBuffer,boost::bind(&BoostSerialized_Client_ASIO::handle_read, this,boost::asio::placeholders::error));
  }
  else
  {
	  std::cout << "Connection Failed" << std::endl;
  }
}


void BoostSerialized_Client_ASIO::handle_read(const boost::system::error_code& error)
{
  if (!error)
  {
	  {
		  boost::unique_lock< boost::shared_mutex > lock(*(mainReceiveBuffer_.Owner_Mutex));
		  mainReceiveBuffer_.add(temporaryReceiveBuffer);
	  }
	 std::cout << "Reading successfull[" << temporaryReceiveBuffer.get().size() << ":" << mainReceiveBuffer_.get().size() << "]" << std::endl;
	temporaryReceiveBuffer.clear();
	std::cout << "Reading again" << std::endl;
	connection_.async_read(temporaryReceiveBuffer,boost::bind(&BoostSerialized_Client_ASIO::handle_read, this,boost::asio::placeholders::error));
  }
  else
  {
	std::cout << "Reading failed :" << error.message() << std::endl;
    do_close();
  }
}

commResult BoostSerialized_Client_ASIO::send(DataContainer& value)
{
	boost::unique_lock< boost::shared_mutex > lock(*value.Owner_Mutex);
	std::cout << "Writing [" << value.get().size()  << "]" << std::endl;
	connection_.async_write(value, boost::bind(&BoostSerialized_Client_ASIO::handle_send, this,boost::asio::placeholders::error, boost::ref(value)));
}
commResult BoostSerialized_Client_ASIO::receive(DataContainer& value)
{

}

void BoostSerialized_Client_ASIO::handle_send(const boost::system::error_code& error,DataContainer &value)
{
  if (!error)
  {
	  boost::unique_lock< boost::shared_mutex > lock(*value.Owner_Mutex);
	  std::cout << "Writing from[" << value.get().size()   << "] Successful" << std::endl;
	  value.reset();
	  value.set_work_in_progress(false);
  }
  else
  {
	  std::cout << "Writing failed :" << error.message() << std::endl;
    do_close();
  }
}

void BoostSerialized_Client_ASIO::do_close()
{
  connection_.socket().close();
}
BoostSerialized_Client_ASIO::~BoostSerialized_Client_ASIO(){
//	io_service_.post([&](){acceptor.cancel();})
//	io_service_.stop();
//	thread_asio.interrupt();
	thread_asio.join();
}






}
