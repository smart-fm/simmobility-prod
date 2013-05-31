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
	static int currTicks = 0;
	if(currTicks%2 == 0){
		ProcessMessages();
	}
	else if(currTicks%2 == 1){
		UpdateMessages();
	}

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

void FMODController::ProcessMessages()
{
	MessageList Requests = CollectRequest();
	connectPoint->pushMessage(Requests);

	MessageList retCols;
	MessageList messages = connectPoint->popMessage();
	while( messages.size()>0 )
	{
		std::string str = messages.front();
		messages.pop();

		int msgId = Message::GetMessageID(str);
		if(msgId == 2){
			HandleVehicleInit(str);
		}
		else if(msgId == 5){
			MessageList ret = HandleOfferMessage(str);
			retCols = retCols+ret;
		}
		else if(msgId == 7){
			MessageList ret = HandleConfirmMessage(str);
			retCols = retCols+ret;
		}
		else if(msgId == 9){
			HandleScheduleMessage(str);
		}
	}

	connectPoint->pushMessage(retCols);
}

void FMODController::UpdateMessages()
{
	MessageList retCols, ret;

	ret = CollectVehStops();
	retCols = retCols+ret;
	ret = CollectVehPos();
	retCols = retCols+ret;
	ret = CollectLinkTravelTime();
	retCols = retCols+ret;

	connectPoint->pushMessage(retCols);
}
MessageList FMODController::CollectVehStops()
{
	MessageList msgs;
	return msgs;
}
MessageList FMODController::CollectVehPos()
{
	MessageList msgs;
	return msgs;
}
MessageList FMODController::CollectLinkTravelTime()
{
	MessageList msgs;
	return msgs;
}

MessageList FMODController::CollectRequest()
{
	MessageList msgs;
	return msgs;
}
MessageList FMODController::HandleOfferMessage(std::string msg)
{
	MessageList msgs;
	return msgs;
}
MessageList FMODController::HandleConfirmMessage(std::string msg)
{
	MessageList msgs;
	return msgs;
}

void FMODController::HandleScheduleMessage(std::string msg)
{

}

void FMODController::HandleVehicleInit(std::string msg)
{

}


}

} /* namespace sim_mob */
