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
boost::asio::io_service FMODController::io_service;

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
	StopClientService();
}

bool FMODController::frame_init(timeslice now)
{
	StartClientService();
	return true;
}

Entity::UpdateStatus FMODController::frame_tick(timeslice now)
{
	frameTicks++;
	unsigned int curTickMS = (frameTicks)*ConfigParams::GetInstance().baseGranMS;

	if(frameTicks%2 == 0){
		ProcessMessages();
	}
	if(curTickMS%updateTiming == 0){
		UpdateMessages();
	}

	return Entity::UpdateStatus::Continue;
}

void FMODController::frame_output(timeslice now)
{

}

bool FMODController::InsertFMODItems(const std::string& personID, TripChainItem* item)
{
	all_items[personID] = item;
	return true;
}

bool FMODController::StartClientService()
{
	bool ret = false;
	ret = connectPoint->ConnectToServer(ipAddress, port);

	if(ret)	{
		boost::thread bt( boost::bind(&boost::asio::io_service::run, &io_service) );
		std::cout << "FMOD communication success" << std::endl;

		// for initialization
		Msg_Initialize request;
		request.messageID_ = 1;
		request.map_type = "osm";
		request.map_file = "cityhall/cityhall.osm";
		request.version = 1;
		DailyTime startTm = ConfigParams::GetInstance().simStartTime;
		request.start_time = startTm.toString();
		std::string msg = request.BuildToString();
		std::cout << msg << std::endl;
		connectPoint->pushMessage(msg);
		connectPoint->Flush();
	}
	else {
		std::cout << "FMOD communication failed" << std::endl;
	}

	return ret;
}

void FMODController::StopClientService()
{
	connectPoint->Stop();
	io_service.stop();
}

void FMODController::ProcessMessages()
{
	MessageList Requests = CollectRequest();
	connectPoint->pushMessage(Requests);

	bool continued=false;
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
			connectPoint->pushMessage(ret);
			continued = true;
		}
		else if(msgId == 7){
			MessageList ret = HandleConfirmMessage(str);
			connectPoint->pushMessage(ret);
			continued = true;
		}
		else if(msgId == 9){
			HandleScheduleMessage(str);
		}

		if(continued)
			messages = connectPoint->popMessage();
	}

	connectPoint->Flush();
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
