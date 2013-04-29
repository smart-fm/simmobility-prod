/*
 * tcp_connection.hpp
 *
 *  Created on: Nov 21, 2012
 *      Author: redheli
 */

#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>


namespace sim_mob {

class CommunicationDataManager;
class ControlManager;



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


}
