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
  static boost::shared_ptr<tcp_connection> create(boost::asio::io_service& io_service);
  boost::asio::ip::tcp::socket& socket();

  void handle_write(const boost::system::error_code& error, size_t bytesTransferred);
  static std::string make_daytime_string();
  void commDone();
  bool sendData(std::string &cmd,std::string data);

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
  void start_accept();

  void handle_accept(boost::shared_ptr<tcp_connection> new_connection, const boost::system::error_code& error);

  boost::asio::ip::tcp::acceptor acceptor_;
};

}
