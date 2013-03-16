/*
 * CommunicationManager.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: redheli
 */

#include <boost/thread.hpp>

#include "CommunicationManager.hpp"
#include "ControlManager.hpp"
#include "tcp_server.hpp"

using boost::asio::ip::tcp;
using namespace sim_mob;



sim_mob::CommunicationManager::CommunicationManager(int port, CommunicationDataManager& comDataMgr, ControlManager& ctrlMgr) :
		comDataMgr(&comDataMgr), ctrlMgr(&ctrlMgr)
{
	listenPort = port;
	simulationDone = false;
	CommDone = true;
}

void sim_mob::CommunicationManager::start()
{
	try
  {
	tcp_server server(io_service,listenPort, *comDataMgr, *ctrlMgr);
	io_service.run();
  }
  catch (std::exception& e)
  {
	std::cerr <<"CommunicationManager::start: "<< e.what() << std::endl;
  }
}

