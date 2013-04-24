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
 * 						ASIO_Impl
 * * ***********************************************************
 */
namespace sim_mob {
ASIO_Impl::ASIO_Impl(std::string host,std::string port,DataContainer &mainReceiveBuffer):
		connection_(io_service_),host_(host),port_(port),mainReceiveBuffer_(mainReceiveBuffer)
{
	init();
}

bool ASIO_Impl::init()
{
	connect();
}

bool ASIO_Impl::connect()
{

	// Resolve the host name into an IP address.
	boost::asio::ip::tcp::resolver resolver(io_service_);
	boost::asio::ip::tcp::resolver::query query(host_, port_);
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
			resolver.resolve(query);
	boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

	std::cout << "Connecting" << std::endl;
	boost::asio::async_connect(connection_.socket(), endpoint_iterator, boost::bind(&ASIO_Impl::handle_connect, this,boost::asio::placeholders::error));
	boost::thread thread_asio = boost::thread(&ASIO_Impl::thread_asio_function,this);
}

bool ASIO_Impl::thread_asio_function() {
	io_service_.run();
}

void ASIO_Impl::close()
{
  io_service_.post(boost::bind(&ASIO_Impl::do_close, this));
}

void ASIO_Impl::handle_connect(const boost::system::error_code& error)
{
  if (!error)
  {
//	  boost::unique_lock< boost::shared_mutex > lock(*mainReceiveBuffer_.Owner_Mutex);
	  std::cout << "Connection Established" << std::endl;
	  std::cout << "Reading" << std::endl;
	  connection_.async_read(temporaryReceiveBuffer,boost::bind(&ASIO_Impl::handle_read, this,boost::asio::placeholders::error));
  }
  else
  {
	  std::cout << "Connection Failed" << std::endl;
  }
}


void ASIO_Impl::handle_read(const boost::system::error_code& error)
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
	connection_.async_read(temporaryReceiveBuffer,boost::bind(&ASIO_Impl::handle_read, this,boost::asio::placeholders::error));
  }
  else
  {
	std::cout << "Reading failed :" << error.message() << std::endl;
    do_close();
  }
}

commResult ASIO_Impl::send(DataContainer& value)
{
	boost::unique_lock< boost::shared_mutex > lock(*value.Owner_Mutex);
	std::cout << "Writing [" << value.get().size()  << "]" << std::endl;
	connection_.async_write(value, boost::bind(&ASIO_Impl::handle_send, this,boost::asio::placeholders::error, boost::ref(value)));
}
commResult ASIO_Impl::receive(DataContainer& value)
{

}

void ASIO_Impl::handle_send(const boost::system::error_code& error,DataContainer &value)
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

void ASIO_Impl::do_close()
{
  connection_.socket().close();
}
ASIO_Impl::~ASIO_Impl(){
//	io_service_.post([&](){acceptor.cancel();})
//	io_service_.stop();
//	thread_asio.interrupt();
	thread_asio.join();
}






}
