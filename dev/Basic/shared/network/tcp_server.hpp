/*
 * tcp_server.hpp
 *
 *  Created on: Nov 21, 2012
 *      Author: redheli
 */

#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

#include "tcp_connection.hpp"
#include <iostream>

namespace sim_mob {

class ControlManager;
class CommunicationDataManager;


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
