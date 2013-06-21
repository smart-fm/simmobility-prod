/*
 * FMODController.cpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include "FMODController.hpp"
#include "Message.hpp"
#include "entities/Person.hpp"
#include <utility>

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
		ProcessMessages(now);
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

void FMODController::CollectPerson()
{
	typedef std::map<Request*, TripChainItem*>::iterator RequestMap;
	for (RequestMap it=all_requests.begin(); it!=all_requests.end(); it++) {

		sim_mob::TripChainItem* tc = it->second;
		tc->personID = boost::lexical_cast<std::string>( it->first->client_id );
		tc->startTime = DailyTime(it->first->departure_time_early)+DailyTime(tc->requestTime*60*1000/2.0);
		std::vector<sim_mob::TripChainItem*>  tcs;
		tcs.push_back(tc);

		sim_mob::Person* person = new sim_mob::Person("FMOD_TripChain", ConfigParams::GetInstance().mutexStategy, tcs);
		all_persons.push_back(person);
	}
}

void FMODController::CollectRequest()
{
	typedef std::map<std::string, TripChainItem*>::iterator TCMapIt;
	for (TCMapIt it=all_items.begin(); it!=all_items.end(); it++) {

		Print() << "Size of tripchain item for person " << it->first << std::endl;
		Trip* tc = dynamic_cast<Trip*>(it->second);

		if(tc == nullptr)
			continue;

		if( tc->sequenceNumber > 0 ){

			float period = tc->endTime.getValue()-tc->startTime.getValue();
			period = period / tc->sequenceNumber;
			DailyTime tm(period);
			DailyTime cur = tc->startTime;
			DailyTime bias = DailyTime(tc->requestTime*60*1000);
			int id = boost::lexical_cast<int>(it->first);
			for(int i=0; i<tc->sequenceNumber; i++){
				Request* request = new Request();
				request->client_id = id+i;
				request->arrival_time_early = "-1";
				request->arrival_time_late = "-1";
				request->origin = tc->fromLocation.getID();
				request->destination = tc->toLocation.getID();

				cur += tm;
				request->departure_time_early = (cur-bias).toString();
				request->departure_time_late = (cur+bias).toString();

				all_requests.insert( std::make_pair(request, tc) );
			}
		}
		else{
			all_items[it->first]=nullptr;
		}
	}
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
		request.map_file = mapFile; //"cityhall/cityhall.osm";
		request.version = 1;
		DailyTime startTm = ConfigParams::GetInstance().simStartTime;
		request.start_time = startTm.toString();
		std::string msg = request.BuildToString();
		std::cout << "FMOD Controller send message :" << msg << std::endl;
		connectPoint->pushMessage(msg);
		connectPoint->pushMessage(msg);
		connectPoint->Flush();

		CollectPerson();
		CollectRequest();
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

void FMODController::ProcessMessages(timeslice now)
{
	MessageList Requests = GenerateRequest(now);
	connectPoint->pushMessage(Requests);
	connectPoint->Flush();

	bool continued = false;
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

MessageList FMODController::GenerateRequest(timeslice now)
{
	MessageList msgs;

	unsigned int curTickMS = (frameTicks)*ConfigParams::GetInstance().baseGranMS;
	DailyTime curr(curTickMS);
	DailyTime base(ConfigParams::GetInstance().simStartTime);
	typedef std::map<Request*, TripChainItem*>::iterator RequestMap;
	for (RequestMap it=all_requests.begin(); it!=all_requests.end(); it++) {

		DailyTime tm(it->first->departure_time_early);
		DailyTime dias(1*3600*1000);
		tm -= dias;
		if( tm.getValue() > (curr+base).getValue() ){
			Msg_Request request;
			request.messageID_ = 4;
			request.request = *it->first;
			msgs.push( request.BuildToString() );
		}
	}
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
	Msg_Schedule msg_request;
	msg_request.CreateMessage( msg );

}

void FMODController::HandleVehicleInit(std::string msg)
{
	msgVehInit.CreateMessage(msg);
}


}

} /* namespace sim_mob */
