/*
 * CommunicationManager.hpp
 *
 *  Created on: Nov 21, 2012
 *      Author: redheli
 */

#pragma once

#include <iostream>
#include <fstream>
#include <ctime>
#include <queue>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/regex.hpp>

namespace sim_mob {

class ConfigParams;
class ControlManager;

class CommunicationDataManager {
public:
	void sendTrafficData(std::string &s);
	void sendRoadNetworkData(std::string &s);
	bool getTrafficData(std::string &s);
	bool getCmdData(std::string &s);
	bool getRoadNetworkData(std::string &s);
	bool isAllTrafficDataOut() { return trafficDataQueue.empty(); }

private:
	CommunicationDataManager();
	std::queue<std::string> trafficDataQueue;
	std::queue<std::string> cmdDataQueue;
	std::queue<std::string> roadNetworkDataQueue;
	boost::mutex trafficDataGuard;
	boost::mutex cmdDataGuard;
	boost::mutex roadNetworkDataGuard;

	//The CommunicationDataManager should be shared for *all* communication, but we want to avoid
	// making it a singleton. For now, that means we put it in ConfigParams, so ConfigParams needs
	// friend access.
	friend class sim_mob::ConfigParams;
};
class CommunicationManager {

public:
	CommunicationManager(int port, CommunicationDataManager& comDataMgr, ControlManager& ctrlMgr);
	void start();
	bool isCommDone() { return CommDone; }
	void setCommDone(bool b) { CommDone = b; }
	void setSimulationDone(bool b) { simulationDone = b; }
	bool isSimulationDone() { return simulationDone; }

private:
	CommunicationDataManager* comDataMgr;
	ControlManager* ctrlMgr;
	boost::asio::io_service io_service;
	int listenPort;

private:
	bool simulationDone;
	bool CommDone;
};

class tcp_connection : public boost::enable_shared_from_this<tcp_connection> {
public:
  static boost::shared_ptr<tcp_connection> create(boost::asio::io_service& io_service)
  {
    return boost::shared_ptr<tcp_connection>(new tcp_connection(io_service));
  }

  boost::asio::ip::tcp::socket& socket()
  {
    return socket_;
  }

  void handle_write(const boost::system::error_code& error, size_t bytesTransferred)
    {
    }
  std::string make_daytime_string()
  {
    std::time_t now = std::time(0);
    return std::ctime(&now);
  }
  void commDone();
  bool sendData(std::string &cmd,std::string data)
  {
	  std::string message_;
		std::string body = "{=" + cmd+"=}{@="+data+"=@}";


		//head size 12
		char msg[20]="\0";

		//TODO: Need to test that this works:
		//sprintf(msg,"{=%08d=}",body.size());
		sprintf(msg,"{=%08zu=}",body.size());

		std::string head=msg;

		message_=head+body;
		boost::system::error_code err;
		 try
		 {
			 boost::asio::async_write(socket_, boost::asio::buffer(message_),
									  boost::bind(&tcp_connection::handle_write,shared_from_this(),
									  boost::asio::placeholders::error,
									  boost::asio::placeholders::bytes_transferred));
			 if(err) {
				 std::cerr<<"start: send error "<<err.message()<<std::endl;
				 return false;
			 }
		 }
		 catch (std::exception& e)
		 {
			 std::cerr <<"start: "<< e.what() << std::endl;
			 return false;
		 }

		 return true;
  }
  bool receiveData(std::string &cmd,std::string &data);
  void trafficDataStart(CommunicationDataManager& comDataMgr);
  void cmdDataStart(CommunicationDataManager& comDataMgr, ControlManager& ctrlMgr);
  void roadNetworkDataStart(CommunicationDataManager& comDataMgr);

private:
  tcp_connection(boost::asio::io_service& io_service)
    : socket_(io_service)
  {}
  boost::asio::ip::tcp::socket socket_;
};

class tcp_server {
public:
  tcp_server(boost::asio::io_service& io_service,int port, CommunicationDataManager& comDataMgr, ControlManager& ctrlMgr);
  bool isClientConnect() { return new_connection->socket().is_open();}

private:
  boost::shared_ptr<tcp_connection> new_connection;
  CommunicationDataManager* comDataMgr;
  ControlManager* ctrlMgr;
  int myPort;
  void start_accept()
  {
    new_connection =
      tcp_connection::create(acceptor_.get_io_service());

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection,
          boost::asio::placeholders::error));
  }

  void handle_accept(boost::shared_ptr<tcp_connection> new_connection, const boost::system::error_code& error)
  {
    if (!error)
    {
    	if(myPort==13333)
    	{
    		new_connection->trafficDataStart(*comDataMgr);
    	}
    	else if(myPort==13334)
		{
			new_connection->cmdDataStart(*comDataMgr, *ctrlMgr);
		}
    	else if(myPort==13335)
		{
			new_connection->roadNetworkDataStart(*comDataMgr);
		}
    	else
    	{
    		std::cout<<"handle_accept: what port it is? "<<myPort<<std::endl;
    	}

    }

    start_accept();
  }

  boost::asio::ip::tcp::acceptor acceptor_;
};

}
