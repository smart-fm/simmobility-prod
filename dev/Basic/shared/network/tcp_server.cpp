/*
 * tcp_server.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: redheli
 */

#include "tcp_server.hpp"

#include <boost/bind.hpp>

using boost::asio::ip::tcp;
using namespace sim_mob;



sim_mob::tcp_server::tcp_server(boost::asio::io_service& io_service,int port, CommunicationDataManager& comDataMgr, ControlManager& ctrlMgr)
  : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), comDataMgr(&comDataMgr), ctrlMgr(&ctrlMgr),
    myPort(port)
{
	start_accept();
}

void sim_mob::tcp_server::start_accept()
{
  new_connection =
    tcp_connection::create(acceptor_.get_io_service());

  acceptor_.async_accept(new_connection->socket(),
      boost::bind(&tcp_server::handle_accept, this, new_connection,
        boost::asio::placeholders::error));
}

void sim_mob::tcp_server::handle_accept(boost::shared_ptr<tcp_connection> new_connection, const boost::system::error_code& error)
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


