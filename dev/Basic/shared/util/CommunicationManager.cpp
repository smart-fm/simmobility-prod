/*
 * CommunicationManager.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: redheli
 */

#include "CommunicationManager.h"
#include <boost/thread.hpp>

sim_mob::CommunicationManager* sim_mob::CommunicationManager::instance = NULL;
//std::queue<std::string> sim_mob::CommunicationManager::dataQueue = NULL;

sim_mob::CommunicationManager* sim_mob::CommunicationManager::GetInstance() {
     if (!instance) {
          instance = new CommunicationManager();
     }
     return instance;
}

sim_mob::CommunicationManager::CommunicationManager() {
	listenPort = 13333;
}

void sim_mob::CommunicationManager::start()
{
	try
  {
	tcp_server server(io_service,listenPort);
	io_service.run();
  }
  catch (std::exception& e)
  {
	std::cerr <<"CommunicationManager::start: "<< e.what() << std::endl;
  }
}

sim_mob::CommunicationManager::~CommunicationManager() {
	// TODO Auto-generated destructor stub
}

