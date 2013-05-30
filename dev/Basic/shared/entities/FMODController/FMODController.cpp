/*
 * FMODController.cpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#include <boost/thread.hpp>
#include "FMODController.hpp"
#include "Message.hpp"

namespace sim_mob {

namespace FMOD
{

FMODController* FMODController::pInstance = nullptr;

void FMODController::RegisterController(int id, const MutexStrategy& mtxStrat)
{
	if(pInstance != nullptr )
		delete pInstance;

	pInstance = new FMODController(id, mtxStrat);
}

FMODController* FMODController::Instance()
{
	return pInstance;
}
FMODController::~FMODController() {
	// TODO Auto-generated destructor stub
}

bool FMODController::frame_init(timeslice now)
{
	StartService();
	return true;
}

Entity::UpdateStatus FMODController::frame_tick(timeslice now)
{
	HandleMessages();
	return Entity::UpdateStatus::Continue;
}

void FMODController::frame_output(timeslice now)
{

}

bool FMODController::CollectFMODAgents(std::vector<sim_mob::Entity*>& all_agents)
{
	Msg_Request request;
	std::string msg = request.BuildToString();
	std::cout << msg << std::endl;
	return true;
}

bool FMODController::StartService()
{
	bool ret = false;
	ret = connectPoint->ConnectToServer(ipAddress, port);
	if(ret)	{
		boost::thread bt( boost::bind(&boost::asio::io_service::run, &io_service) );
	}

	return ret;
}

void FMODController::StopService()
{
	connectPoint->Stop();
	io_service.stop();
}

void FMODController::HandleMessages()
{
	MessageList messages = connectPoint->popMessage();
}

}

} /* namespace sim_mob */
