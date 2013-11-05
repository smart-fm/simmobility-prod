//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * FMODController.cpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include "FMODController.hpp"
#include "JMessage.hpp"
#include "entities/Person.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/Link.hpp"
#include "event/EventCollectionMgr.hpp"
#include <utility>
#include <stdexcept>

namespace sim_mob {

namespace FMOD
{

FMODController* FMODController::pInstance = nullptr;
boost::asio::io_service FMODController::io_service;

void FMODController::RegisterController(int id, const MutexStrategy& mtxStrat)
{
	if(pInstance) {
		delete pInstance;
	}

	pInstance = new FMODController(id, mtxStrat);
}

bool FMODController::InstanceExists()
{
	return pInstance;
}

FMODController* FMODController::Instance()
{
	if (!pInstance) {
		throw std::runtime_error("FMOD controller instance is null.");
	}

	return pInstance;
}
FMODController::~FMODController() {
	// TODO Auto-generated destructor stub
	StopClientService();
}

bool FMODController::frame_init(timeslice now)
{
	return true;
}

Entity::UpdateStatus FMODController::frame_tick(timeslice now)
{
	frameTicks++;
	unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();


	if(frameTicks%2 == 0){
		ProcessMessagesInBlocking(now);
		//ProcessMessages(now);
	}
	if(frameTicks%2 == 1){
		UpdateMessagesInBlocking(now);
		//UpdateMessages(now);
	}

	DispatchPendingAgents(now);

	// test event message transmition system
	/*static int testEvt = 0;
	if( testEvt++ == 20000 ) {

		event::EventCollectionMgr& eventManager = currWorkerProvider->getEventManager();
		event::MessagePtr messagePtr(new event::EventMessage());

		unsigned int agentIds[] = {24, 25, 26, 28 };

		messagePtr->SetEventId(AGENT_INCIDENT_EVENT_ID);
		messagePtr->SetRecipients( agentIds, sizeof(agentIds)/sizeof(unsigned int) );

		eventManager.SendMessage( messagePtr );
	}*/

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
		tc->setPersonID( boost::lexical_cast<std::string>( it->first->clientId ) );
		tc->startTime = DailyTime(it->first->departureTimeEarly); //+DailyTime(tc->requestTime*60*1000/2.0);

		std::vector<sim_mob::TripChainItem*>  tcs;
		tcs.push_back(tc);

		sim_mob::Person* person = new sim_mob::Person("FMOD_TripChain", ConfigManager::GetInstance().FullConfig().mutexStategy(), tcs);
		person->client_id = it->first->clientId;
		all_persons.push_back(person);
		std::cout << "create person id : "  << person->getId() << std::endl ;
		//person->setStartTime( tc->startTime.getValue() );
	}
}

void FMODController::CollectRequest()
{
	typedef std::map<std::string, TripChainItem*>::iterator TCMapIt;
	for (TCMapIt it=all_items.begin(); it!=all_items.end(); it++) {

		Print() << "Size of tripchain item for person " << it->first << std::endl;
		Trip* tc = dynamic_cast<Trip*>(it->second);

		if(tc == nullptr){
			continue;
		}

		if( tc->sequenceNumber > 0 ){

			float period = tc->endTime.getValue()-tc->startTime.getValue();
			period = period / tc->sequenceNumber;
			DailyTime tm(period);
			DailyTime cur = tc->startTime;
			const unsigned int MIN_MS = 60000;
			DailyTime bias = DailyTime(tc->requestTime*MIN_MS);
			int id = boost::lexical_cast<int>(it->first);

			for(int i=0; i<tc->sequenceNumber; i++){
				Request* request = new Request();
				request->clientId = id+i;
				request->arrivalTimeEarly = "-1";
				request->arrivalTimeLate = "-1";
				request->origin = 0;
				if( tc->fromLocation.type_ == WayPoint::NODE )
					request->origin = tc->fromLocation.node_->getID();
				request->destination = 0;
				if(tc->toLocation.type_ == WayPoint::NODE )
					request->destination = tc->toLocation.node_->getID();

				cur += tm;
				request->departureTimeEarly = DailyTime(cur.getValue()-tc->requestTime*60*1000/2).toString();;
				request->departureTimeLate = DailyTime(cur.getValue()+tc->requestTime*60*1000/2).toString();;

				all_requests.insert( std::make_pair(request, tc) );
			}
		}
		else{
			all_items[it->first]=nullptr;
		}
	}
}

void FMODController::Initialize()
{
	CollectRequest();
	CollectPerson();
}


bool FMODController::ConnectFMODService()
{
	bool ret = false;
	ret = connectPoint->ConnectToServer(ipAddress, port);

	if(ret)	{

		boost::thread bt( boost::bind(&boost::asio::io_service::run, &io_service) );
		std::cout << "FMOD communication success" << std::endl;

		// for initialization
		MsgInitialize request;
		request.messageID_ = JMessage::MSG_INITIALIZE;
		request.mapType = "osm";
		request.mapFile = mapFile; //"cityhall/cityhall.osm";
		request.version = 1;
		request.startTime = ConfigManager::GetInstance().FullConfig().simStartTime().toString();
		std::string msg = request.BuildToString();
		std::cout << "FMOD Controller send message :" << msg << std::endl;
		connectPoint->SendMessage(msg);
		//connectPoint->pushMessage(msg);
		connectPoint->Flush();

		std::string message;
		if( !connectPoint->WaitMessageInBlocking(message, waitingseconds)){
			std::cout << "FMOD communication in blocking mode not receive data asap" << std::endl;
		}
		else{
			this->HandleVehicleInit(message);
			isConnectFMODServer = true;
		}
	}
	else {
		std::cout << "FMOD communication failed" << std::endl;
		isConnectFMODServer = false;
	}

	return ret;
}

void FMODController::StopClientService()
{
	connectPoint->Stop();
	io_service.stop();
}

void FMODController::ProcessMessagesInBlocking(timeslice now)
{
	if(!isConnectFMODServer)
		return;

	MessageList requests = GenerateRequest(now);

	while( requests.size() > 0 ){
		std::string message = requests.front();
		connectPoint->SendMessage(message);
		requests.pop();

		while( true ){

			connectPoint->Flush();
			if( !connectPoint->WaitMessageInBlocking(message, waitingseconds)){
				std::cout << "FMOD communication in blocking mode not receive data asap" << std::endl;
				break;
			}

			int msgId = JMessage::GetMessageID(message);
			if(msgId == JMessage::MSG_OFFER ){
				MessageList ret = HandleOfferMessage(message);
				connectPoint->SendMessage(ret);
				continue;
			}
			else if(msgId == JMessage::MSG_CONFIRMATION ){
				MessageList ret = HandleConfirmMessage(message);
				connectPoint->SendMessage(ret);
				continue;
			}
			else if(msgId == JMessage::MSG_SCHEDULE ){
				HandleScheduleMessage(message);
				break;
			}
		}
	}
}

void FMODController::ProcessMessages(timeslice now)
{
	if(!isConnectFMODServer)
		return;

	MessageList Requests = GenerateRequest(now);
	connectPoint->SendMessage(Requests);
	connectPoint->Flush();

	bool continued = false;
	MessageList messages = connectPoint->GetMessage();
	while( messages.size()>0 )
	{
		std::string str = messages.front();
		messages.pop();

		int msgId = JMessage::GetMessageID(str);
		if(msgId == JMessage::MSG_SIMULATION_SETTINGS ){
			HandleVehicleInit(str);
		}
		else if(msgId == JMessage::MSG_OFFER ){
			MessageList ret = HandleOfferMessage(str);
			connectPoint->SendMessage(ret);
			continued = true;
		}
		else if(msgId == JMessage::MSG_CONFIRMATION ){
			MessageList ret = HandleConfirmMessage(str);
			connectPoint->SendMessage(ret);
			continued = true;
		}
		else if(msgId == JMessage::MSG_SCHEDULE ){
			HandleScheduleMessage(str);
		}

		if(continued)
			messages = connectPoint->GetMessage();
	}

	connectPoint->Flush();
}

void FMODController::UpdateMessagesInBlocking(timeslice now)
{
	if(!isConnectFMODServer)
		return;

	unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();

	std::string message;
	MessageList ret = CollectVehStops();
	connectPoint->SendMessage(ret);
	connectPoint->WaitMessageInBlocking(message, waitingseconds);
	if(JMessage::GetMessageID(message)!= JMessage::MSG_ACK ){
		std::cout << "FMOD Controller not receive correct acknowledge message" << std::endl;
	}

	if(curTickMS%100 == 0){
		ret = CollectVehPos();
		connectPoint->SendMessage(ret);
		connectPoint->Flush();
		connectPoint->WaitMessageInBlocking(message, waitingseconds);
		if(JMessage::GetMessageID(message)!= JMessage::MSG_ACK ){
			std::cout << "FMOD Controller not receive correct acknowledge message" << std::endl;
		}
	}

	if(curTickMS%updateTiming == 0){
		ret = CollectLinkTravelTime();
		connectPoint->SendMessage(ret);
		connectPoint->WaitMessageInBlocking(message, waitingseconds);
		if(JMessage::GetMessageID(message)!= JMessage::MSG_ACK ){
			std::cout << "FMOD Controller not receive correct acknowledge message" << std::endl;
		}
	}


}

void FMODController::UpdateMessages(timeslice now)
{
	if(!isConnectFMODServer)
		return;

	unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();

	MessageList ret = CollectVehStops();
	connectPoint->SendMessage(ret);

	if(curTickMS%100 == 0){
		ret = CollectVehPos();
		connectPoint->SendMessage(ret);
	}

	if(curTickMS%updateTiming == 0){
		ret = CollectLinkTravelTime();
		connectPoint->SendMessage(ret);
	}

	connectPoint->Flush();
}

MessageList FMODController::CollectVehStops()
{
	MessageList msgs;
	for(std::vector<Agent*>::iterator it=all_children.begin(); it!=all_children.end(); it++){
		Person* person = dynamic_cast<sim_mob::Person*>(*it);
		if(person){

			int client_id = person->client_id;
			Role* driver = person->getRole();

			if(driver){
				std::vector<sim_mob::BufferedBase*> res = driver->getDriverInternalParams();
				if(res.size() == 6){

					sim_mob::BufferedBase* tst1 = res[0];
					Shared<std::string>* stop_event_time = dynamic_cast<Shared<std::string>* >(res[0]);
					Shared<int>* stop_event_type = dynamic_cast<Shared<int>* >(res[1]);
					Shared<int>* stop_event_scheduleid = dynamic_cast<Shared<int>* >(res[2]);
					Shared<int>* stop_event_nodeid = dynamic_cast<Shared<int>* >(res[3]);
					Shared< std::vector<int> >* stop_event_lastBoardingPassengers = dynamic_cast<Shared< std::vector<int> >* >(res[4]);
					Shared< std::vector<int> >* stop_event_lastAlightingPassengers = dynamic_cast<Shared< std::vector<int> >* >(res[5]);

					if(stop_event_time && stop_event_type && stop_event_scheduleid && stop_event_lastBoardingPassengers && stop_event_lastAlightingPassengers){
						if(stop_event_type->get() >= 0 ){

							MsgVehicleStop msg_stop;
							msg_stop.messageID_ = JMessage::MSG_VEHICLESTOP;

							unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();
							DailyTime curr(curTickMS);
							DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());
							DailyTime start(curr.getValue()+base.getValue());

							msg_stop.current_time = start.toString();
							msg_stop.event_type = stop_event_type->get();
							msg_stop.schedule_id = boost::lexical_cast<std::string>(stop_event_scheduleid->get());
							msg_stop.stop_id = boost::lexical_cast<std::string>(stop_event_nodeid->get());
							msg_stop.vehicle_id = boost::lexical_cast<std::string>(client_id);
							msg_stop.aligting_passengers = *(stop_event_lastBoardingPassengers);
							msg_stop.boarding_passengers = *(stop_event_lastAlightingPassengers);

							std::string msg = msg_stop.BuildToString();
							msgs.push(msg);

							int type = stop_event_type->get();
							stop_event_type->set(-1);
						}
					}

				}
			}
		}
	}
	return msgs;
}

MessageList FMODController::CollectVehPos()
{
	MessageList msgs;

	for(std::vector<Agent*>::iterator it=all_children.begin(); it!=all_children.end(); it++){
		Person* person = dynamic_cast<sim_mob::Person*>(*it);
		if(person){
			MsgVehiclePos msg_pos;
			msg_pos.messageID_ = JMessage::MSG_VEHICLEPOS;
			unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();

			DailyTime curr(curTickMS);
			DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());
			DailyTime start(curr.getValue()+base.getValue());

			msg_pos.current_time = start.toString();
			msg_pos.vehicle_id = person->client_id;
			msg_pos.latitude = person->xPos.get();
			msg_pos.longtitude = person->yPos.get();

			std::string msg = msg_pos.BuildToString();
			msgs.push(msg);
		}
	}

	return msgs;
}

MessageList FMODController::CollectLinkTravelTime()
{
	MessageList msgs;

	for(std::vector<Agent*>::iterator it=all_children.begin(); it!=all_children.end(); it++){

		const std::map<double, travelStats>& travelStatsMap = (*it)->travelStatsMap.get();
		for(std::map<double, travelStats>::const_iterator itTravel=travelStatsMap.begin(); itTravel!=travelStatsMap.end(); itTravel++ ){

			double travelTime = (itTravel->first) - (itTravel->second).linkEntryTime_;
			std::map<const Link*, travelTimes>::iterator itTT = LinkTravelTimesMap.find((itTravel->second).link_);

			if (itTT != LinkTravelTimesMap.end())
			{
				itTT->second.agentCount = itTT->second.agentCount + 1;
				itTT->second.linkTravelTime = itTT->second.linkTravelTime + travelTime;
			}
			else{
				travelTimes tTimes(travelTime, 1);
				LinkTravelTimesMap.insert(std::make_pair((itTravel->second).link_, tTimes));
			}
		}
	}

	unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();
	DailyTime curr(curTickMS);
	DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());
	DailyTime start(curr.getValue()+base.getValue());

	MsgLinkTravel msg_travel;
	msg_travel.currentTime = start.toString();
	msg_travel.messageID_ = JMessage::MSG_LINKTRAVELUPADTE;
	for(std::map<const sim_mob::Link*, travelTimes>::iterator itTT=LinkTravelTimesMap.begin(); itTT!=LinkTravelTimesMap.end(); itTT++){
		MsgLinkTravel::LINK travel;
		(itTT->first)->getStart()->getID();
		travel.node1Id = (itTT->first)->getStart()->getID();
		travel.node2Id = (itTT->first)->getEnd()->getID();
		travel.wayId = (itTT->first)->getLinkId();
		travel.travelTime = (itTT->second).linkTravelTime;
		msg_travel.links.push_back(travel);
	}

	std::string msg = msg_travel.BuildToString();
	msgs.push(msg);

	return msgs;
}

MessageList FMODController::GenerateRequest(timeslice now)
{
	MessageList msgs;

	unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();
	DailyTime curr(curTickMS);
	DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());
	typedef std::map<Request*, TripChainItem*>::iterator RequestMap;
	for (RequestMap it=all_requests.begin(); it!=all_requests.end(); it++) {

		DailyTime tm(it->first->departureTimeEarly);
		tm.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime());
		DailyTime dias(1*3600*1000);

		if( tm.getValue() > (curr.getValue()-dias.getValue() )){
			MsgRequest request;
			request.messageID_ = JMessage::MSG_REQUEST;
			request.request = *it->first;
			msgs.push( request.BuildToString() );
		}
	}

	return msgs;
}
MessageList FMODController::HandleOfferMessage(std::string& msg)
{
	MessageList msgs;

	MsgOffer msg_offer;
	msg_offer.CreateMessage(msg);

	unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();
	DailyTime curr(curTickMS);
	DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());

	MsgAccept msg_accept;
	msg_accept.messageID_ = JMessage::MSG_ACCEPT;
	msg_accept.clientId = msg_offer.clientId;
	msg_accept.scheduleId = msg_offer.offers[0].schduleId;
	msg_accept.arrivalTime = msg_offer.offers[0].arivalTimeEarly;
	msg_accept.departureTime = msg_offer.offers[0].departureTimeEarly;
	msg_accept.currentTime = (curr+base).toString();
	std::string str = msg_accept.BuildToString();

	msgs.push( msg_accept.BuildToString() );

	return msgs;
}
MessageList FMODController::HandleConfirmMessage(std::string& msg)
{
	MessageList msgs;

	MsgConfirmation msg_confirm;
	msg_confirm.CreateMessage(msg);

	JMessage msg_fetch;
	msg_fetch.messageID_ = JMessage::MSG_SCHEDULE_FETCH;
	std::string str = msg_fetch.BuildToString();

	msgs.push(str);

	return msgs;
}

void FMODController::HandleScheduleMessage(std::string& msg)
{
	MsgSchedule msg_request;
	msg_request.CreateMessage( msg );

	FMODSchedule* schedule = new FMODSchedule();
	sim_mob::Node* startNode=nullptr;
	sim_mob::Node* endNode=nullptr;
	for(std::vector<MsgSchedule::ROUTE>::iterator it=msg_request.routes.begin(); it!=msg_request.routes.end(); it++){
		const StreetDirectory& stdir = StreetDirectory::instance();
		int id = boost::lexical_cast<int>( (*it).id );
		sim_mob::Node* node = const_cast<sim_mob::Node*>(stdir.getNode(id));
		if(node){
			schedule->routes.push_back(node);
			if(startNode==nullptr){
				startNode = node;
			}
			endNode = node;
		}
	}

	DailyTime start(0);
	for(std::vector<MsgSchedule::STOP>::iterator it=msg_request.stop_schdules.begin(); it!=msg_request.stop_schdules.end(); it++){
		FMODSchedule::STOP stop;
		stop.stop_id = boost::lexical_cast<int>( (*it).stop_id );
		stop.schedule_id = boost::lexical_cast<int>( msg_request.schedule_id );
		stop.dwell_time = 0;
		for(std::vector<std::string>::iterator itP=(*it).alightingpassengers.begin(); itP!=(*it).alightingpassengers.end(); itP++){
			stop.alightingpassengers.push_back( boost::lexical_cast<int>( (*itP) ) );
		}
		for(std::vector<std::string>::iterator itP=(*it).boardingpassengers.begin(); itP!=(*it).boardingpassengers.end(); itP++){
			stop.boardingpassengers.push_back( boost::lexical_cast<int>( (*itP) ) );
		}
		schedule->stop_schdules.push_back(stop);
		if(start.getValue()==0){
			start = DailyTime( (*it).arrival_time );
		}
	}

	sim_mob::TripChainItem* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", startNode, "node", endNode, "node");
	SubTrip subTrip("-1", "Trip", 0, -1, start, DailyTime(), startNode, "node", endNode, "node", "Car");
	subTrip.schedule = schedule;
	((Trip*)tc)->addSubTrip(subTrip);

	std::vector<sim_mob::TripChainItem*>  tcs;
	tcs.push_back(tc);

	if( parkingCoord.remove(msg_request.vehicle_id) ){
		sim_mob::Person* person = new sim_mob::Person("FMOD_TripChain", ConfigManager::GetInstance().FullConfig().mutexStategy(), tcs);
		person->client_id = msg_request.vehicle_id ;
		person->parentEntity = this;
		all_drivers.push_back(person);
		parkingCoord.enterTo(person->originNode.node_, person);
	}

}

void FMODController::unregisteredChild(Entity* child)
{
	Agent* agent = dynamic_cast<Agent*>(child);
	if(agent)
	{
		std::vector<Agent*>::iterator it = std::find(all_children.begin(), all_children.end(), agent);
		if (it != all_children.end() ) {
			all_children.erase(it);

			sim_mob::Person* person = dynamic_cast<Person*>(agent);
			parkingCoord.enterTo(person->originNode.node_, person);
		}
	}
}

void FMODController::HandleVehicleInit(std::string& msg)
{
	MsgVehicleInit msg_init;
	msg_init.CreateMessage(msg);

	for(std::vector<MsgVehicleInit::SUPPLY>::iterator it=msg_init.vehicles.begin(); it!=msg_init.vehicles.end(); it++){

		const StreetDirectory& stdir = StreetDirectory::instance();
		Node* node = const_cast<Node*>( stdir.getNode( (*it).nodeId ) );
		if(node != nullptr){
			DailyTime start(0);
			sim_mob::TripChainItem* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", node, "node", node, "node");
			std::vector<sim_mob::TripChainItem*>  tcs;
			tcs.push_back(tc);

			sim_mob::Person* person = new sim_mob::Person("FMOD_TripChain", ConfigManager::GetInstance().FullConfig().mutexStategy(), tcs);
			person->parentEntity = this;
			person->client_id = (*it).vehicleId;

			parkingCoord.enterTo(node, person);
		}
	}

}

void FMODController::DispatchPendingAgents(timeslice now)
{
	unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();
	std::vector<Agent*>::iterator it=all_persons.begin();
	while(it!=all_persons.end()){
		if( (*it)->getStartTime() < curTickMS ){
			this->currWorkerProvider->scheduleForBred( (*it) );
			it = all_persons.erase(it);
		}
		else{
			it++;
		}
	}

	it = all_drivers.begin();
	while(it!=all_drivers.end()){
		if( (*it)->getStartTime() < curTickMS ){
			this->currWorkerProvider->scheduleForBred( (*it) );
			all_children.push_back((*it));
			it = all_drivers.erase(it);
		}
		else{
			++it;
		}
	}


	/*
	// for testing
	static int kk = 0;
	if( kk++ == 0){

		const StreetDirectory& stdir = StreetDirectory::instance();
		sim_mob::Node* node1 = const_cast<sim_mob::Node*>(stdir.getNode(45666));
		sim_mob::Node* node2 = const_cast<sim_mob::Node*>(stdir.getNode(58950));
		sim_mob::Node* node3 = const_cast<sim_mob::Node*>(stdir.getNode(75956));
		sim_mob::Node* node4 = const_cast<sim_mob::Node*>(stdir.getNode(66508));
		sim_mob::Node* node5 = const_cast<sim_mob::Node*>(stdir.getNode(93730));

		DailyTime start(5*60*1000);
		start += ConfigParams::GetInstance().simStartTime;
		sim_mob::TripChainItem* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", node1, "node", node4, "node");

		FMODSchedule* schedule = new FMODSchedule();
		schedule->routes.push_back(node1);
		schedule->routes.push_back(node2);
		schedule->routes.push_back(node3);
		schedule->routes.push_back(node4);
		schedule->routes.push_back(node5);

		FMODSchedule::STOP stop;
		stop.stop_id = 58950;
		stop.dwell_time = 0;
		stop.schedule_id = 222;
		stop.boardingpassengers.push_back(26);
		stop.boardingpassengers.push_back(27);
		stop.boardingpassengers.push_back(28);
		stop.boardingpassengers.push_back(29);
		schedule->stop_schdules.push_back(stop);

		FMODSchedule::STOP stpB;
		stpB.stop_id = 75956;
		stpB.dwell_time = 0;
		stpB.schedule_id = 222;
		stpB.alightingpassengers.push_back(26);
		stpB.alightingpassengers.push_back(27);
		stpB.alightingpassengers.push_back(28);
		stpB.alightingpassengers.push_back(29);
		schedule->stop_schdules.push_back(stpB);

		std::cout << "stop schedule size is :" << schedule->stop_schdules.size() << std::endl;

		SubTrip subTrip("-1", "Trip", 0, -1, start, DailyTime(), node1, "node", node4, "node", "Car");
		subTrip.schedule = schedule;
		((Trip*)tc)->addSubTrip(subTrip);

		std::vector<sim_mob::TripChainItem*>  tcs;
		tcs.push_back(tc);
		//tcs.push_back(tc);

		//Activity* waiting = new Activity();
		//waiting->personID = 1;
		//waiting->itemType = TripChainItem::IT_ACTIVITY;
		//waiting->sequenceNumber = 1;
		//waiting->description = "waiting";
		//waiting->isPrimary = false;
		//waiting->isFlexible = false;
		//waiting->isMandatory = false;
		//waiting->location = node5;
		//waiting->locationType = TripChainItem::LT_NODE;
		//waiting->startTime = start;
		//waiting->endTime = DailyTime(10*60*1000)+ConfigParams::GetInstance().simStartTime;;
		//tcs.push_back(waiting);

		sim_mob::Person* person = new sim_mob::Person("FMOD_TripChain", ConfigParams::GetInstance().mutexStategy, tcs);
		person->parentEntity = this;
		person->client_id = 101;
		all_drivers.push_back(person);
	}

	*/

}

}

} /* namespace sim_mob */
