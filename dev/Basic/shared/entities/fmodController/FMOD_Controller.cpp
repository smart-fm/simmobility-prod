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
#include "FMOD_Controller.hpp"
#include "FMOD_Message.hpp"
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

FMOD_Controller* FMOD_Controller::pInstance = nullptr;
boost::asio::io_service FMOD_Controller::ioService;

void FMOD_Controller::registerController(int id, const MutexStrategy& mtxStrat)
{
	if(pInstance) {
		delete pInstance;
	}

	pInstance = new FMOD_Controller(id, mtxStrat);
}

bool FMOD_Controller::instanceExists()
{
	return pInstance;
}

FMOD_Controller* FMOD_Controller::instance()
{
	if (!pInstance) {
		throw std::runtime_error("Fmod controller instance is null.");
	}

	return pInstance;
}
FMOD_Controller::~FMOD_Controller() {
	// TODO Auto-generated destructor stub
	stopClientService();
}

bool FMOD_Controller::frame_init(timeslice now)
{
	return true;
}

Entity::UpdateStatus FMOD_Controller::frame_tick(timeslice now)
{
	frameTicks++;
	unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();


	if(frameTicks%2 == 0){
		processMessagesInBlocking(now);
	}
	else if(frameTicks%2 == 1){
		updateMessagesInBlocking(now);
	}

	dispatchPendingAgents(now);

	return Entity::UpdateStatus::Continue;
}

void FMOD_Controller::frame_output(timeslice now)
{

}

bool FMOD_Controller::insertFmodItems(const std::string& personID, TripChainItem* item)
{
	allItems[personID] = item;
	return true;
}

void FMOD_Controller::collectPerson()
{
	std::map<Request*, TripChainItem*>::iterator it;
	for (it=allRequests.begin(); it!=allRequests.end(); it++) {

		sim_mob::TripChainItem* tc = it->second;
		tc->setPersonID( boost::lexical_cast<std::string>( it->first->clientId ) );
		tc->startTime = DailyTime(it->first->departureTimeEarly); //+DailyTime(tc->requestTime*60*1000/2.0);

		std::vector<sim_mob::TripChainItem*>  tcs;
		tcs.push_back(tc);

		sim_mob::Person* person = new sim_mob::Person("FMOD_TripChain", ConfigManager::GetInstance().FullConfig().mutexStategy(), tcs);
		person->client_id = it->first->clientId;
		allPersons.push_back(person);
	}
}

void FMOD_Controller::collectRequest()
{
	std::map<std::string, TripChainItem*>::iterator it;
	for (it=allItems.begin(); it!=allItems.end(); it++) {

		Print() << "Size of tripchain item for person " << it->first << std::endl;
		Trip* trip = dynamic_cast<Trip*>(it->second);

		if(trip == nullptr){
			continue;
		}

		if( trip->sequenceNumber > 0 ){

			float period = trip->endTime.getValue()-trip->startTime.getValue();
			period = period / trip->sequenceNumber;
			DailyTime tm(period);
			DailyTime cur = trip->startTime;
			const unsigned int MIN_MS = 60000;
			DailyTime bias = DailyTime(trip->requestTime*MIN_MS);
			int id = boost::lexical_cast<int>(it->first);

			for(int i=0; i<trip->sequenceNumber; i++){
				Request* request = new Request();
				request->clientId = id+i;
				request->arrivalTimeEarly = "-1";
				request->arrivalTimeLate = "-1";
				request->origin = 0;
				if( trip->fromLocation.type_ == WayPoint::NODE )
					request->origin = trip->fromLocation.node_->getID();
				request->destination = 0;
				if(trip->toLocation.type_ == WayPoint::NODE )
					request->destination = trip->toLocation.node_->getID();

				cur += tm;
				const int MIN_MS = 60*1000;
				request->departureTimeEarly = DailyTime(cur.getValue()-trip->requestTime*MIN_MS/2).toString();;
				request->departureTimeLate = DailyTime(cur.getValue()+trip->requestTime*MIN_MS/2).toString();;

				allRequests.insert( std::make_pair(request, trip) );
			}
		}
		else{
			allItems[it->first]=nullptr;
		}
	}
}

void FMOD_Controller::initialize()
{
	collectRequest();
	collectPerson();
}


bool FMOD_Controller::connectFmodService()
{
	bool ret = connectPoint->connectToServer(ipAddress, port);

	if(ret)	{

		boost::thread bt( boost::bind(&boost::asio::io_service::run, &ioService) );

		// for initialization
		MsgInitialize request;
		request.messageID_ = FMOD_Message::MSG_INITIALIZE;
		request.mapType = "osm";
		request.mapFile = mapFile;
		request.version = 1;
		request.startTime = ConfigManager::GetInstance().FullConfig().simStartTime().toString();
		std::string msg = request.buildToString();
		connectPoint->sendMessage(msg);
		connectPoint->flush();

		std::string message;
		if( !connectPoint->waitMessageInBlocking(message, waitingseconds)){
			std::cout << "FMOD communication in blocking mode not receive data asap" << std::endl;
		}
		else{
			handleVehicleInit(message);
			isConnectFmodServer = true;
		}
	}
	else {
		std::cout << "FMOD communication failed" << std::endl;
		isConnectFmodServer = false;
	}

	return ret;
}

void FMOD_Controller::stopClientService()
{
	connectPoint->stop();
	ioService.stop();
}

void FMOD_Controller::processMessagesInBlocking(timeslice now)
{
	if(!isConnectFmodServer)
		return;

	MessageList requests = generateRequest(now);

	while( requests.size() > 0 ){
		std::string message = requests.front();
		connectPoint->sendMessage(message);
		requests.pop();

		while( true ){

			connectPoint->flush();
			if( !connectPoint->waitMessageInBlocking(message, waitingseconds)){
				std::cout << "FMOD communication in blocking mode not receive data asap" << std::endl;
				break;
			}

			int msgId = FMOD_Message::analyzeMessageID(message);
			if(msgId == FMOD_Message::MSG_OFFER ){
				MessageList ret = handleOfferMessage(message);
				connectPoint->sendMessage(ret);
				continue;
			}
			else if(msgId == FMOD_Message::MSG_CONFIRMATION ){
				MessageList ret = handleConfirmMessage(message);
				connectPoint->sendMessage(ret);
				continue;
			}
			else if(msgId == FMOD_Message::MSG_SCHEDULE ){
				handleScheduleMessage(message);
				break;
			}
		}
	}
}

void FMOD_Controller::processMessages(timeslice now)
{
	if(!isConnectFmodServer)
		return;

	MessageList Requests = generateRequest(now);
	connectPoint->sendMessage(Requests);
	connectPoint->flush();

	bool continued = false;
	MessageList messages = connectPoint->getMessage();
	while( messages.size()>0 )
	{
		std::string str = messages.front();
		messages.pop();

		int msgId = FMOD_Message::analyzeMessageID(str);
		if(msgId == FMOD_Message::MSG_SIMULATION_SETTINGS ){
			handleVehicleInit(str);
		}
		else if(msgId == FMOD_Message::MSG_OFFER ){
			MessageList ret = handleOfferMessage(str);
			connectPoint->sendMessage(ret);
			continued = true;
		}
		else if(msgId == FMOD_Message::MSG_CONFIRMATION ){
			MessageList ret = handleConfirmMessage(str);
			connectPoint->sendMessage(ret);
			continued = true;
		}
		else if(msgId == FMOD_Message::MSG_SCHEDULE ){
			handleScheduleMessage(str);
		}

		if(continued)
			messages = connectPoint->getMessage();
	}

	connectPoint->flush();
}

void FMOD_Controller::updateMessagesInBlocking(timeslice now)
{
	if(!isConnectFmodServer)
		return;

	unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();

	std::string message;
	MessageList ret = collectVehStops();
	connectPoint->sendMessage(ret);
	connectPoint->waitMessageInBlocking(message, waitingseconds);
	if(FMOD_Message::analyzeMessageID(message)!= FMOD_Message::MSG_ACK ){
		std::cout << "Fmod Controller not receive correct acknowledge message" << std::endl;
		return;
	}

	if(curTickMS%updateTiming == 0){
		ret = collectVehPos();
		connectPoint->sendMessage(ret);
		connectPoint->flush();
		connectPoint->waitMessageInBlocking(message, waitingseconds);
		if(FMOD_Message::analyzeMessageID(message)!= FMOD_Message::MSG_ACK ){
			std::cout << "Fmod Controller not receive correct acknowledge message" << std::endl;
			return;
		}
	}

	if(curTickMS%updateTiming == 0){
		ret = collectLinkTravelTime();
		connectPoint->sendMessage(ret);
		connectPoint->waitMessageInBlocking(message, waitingseconds);
		if(FMOD_Message::analyzeMessageID(message)!= FMOD_Message::MSG_ACK ){
			std::cout << "Fmod Controller not receive correct acknowledge message" << std::endl;
			return;
		}
	}


}

void FMOD_Controller::updateMessages(timeslice now)
{
	if(!isConnectFmodServer)
		return;

	unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();

	MessageList ret = collectVehStops();
	connectPoint->sendMessage(ret);

	if(curTickMS%updateTiming == 0){
		ret = collectVehPos();
		connectPoint->sendMessage(ret);
	}

	if(curTickMS%updateTiming == 0){
		ret = collectLinkTravelTime();
		connectPoint->sendMessage(ret);
	}

	connectPoint->flush();
}

MessageList FMOD_Controller::collectVehStops()
{
	MessageList msgs;
	for(std::vector<Agent*>::iterator it=allChildren.begin(); it!=allChildren.end(); it++){
		Person* person = dynamic_cast<sim_mob::Person*>(*it);
		if(person){

			int client_id = person->client_id;
			Role* driver = person->getRole();

			if(driver){
				std::vector<sim_mob::BufferedBase*> res = driver->getDriverInternalParams();
				if(res.size() > 0 ){

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
							msg_stop.messageID_ = FMOD_Message::MSG_VEHICLESTOP;

							unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();
							DailyTime curr(curTickMS);
							DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());
							DailyTime start(curr.getValue()+base.getValue());

							msg_stop.currentTime = start.toString();
							msg_stop.eventType = stop_event_type->get();
							msg_stop.scheduleId = boost::lexical_cast<std::string>(stop_event_scheduleid->get());
							msg_stop.stopId = boost::lexical_cast<std::string>(stop_event_nodeid->get());
							msg_stop.vehicleId = boost::lexical_cast<std::string>(client_id);
							msg_stop.aligtingPassengers = *(stop_event_lastBoardingPassengers);
							msg_stop.boardingPassengers = *(stop_event_lastAlightingPassengers);

							std::string msg = msg_stop.buildToString();
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

MessageList FMOD_Controller::collectVehPos()
{
	MessageList msgs;

	for(std::vector<Agent*>::iterator it=allChildren.begin(); it!=allChildren.end(); it++){
		Person* person = dynamic_cast<sim_mob::Person*>(*it);
		if(person){
			MsgVehiclePos msg_pos;
			msg_pos.messageID_ = FMOD_Message::MSG_VEHICLEPOS;
			unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();

			DailyTime curr(curTickMS);
			DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());
			DailyTime start(curr.getValue()+base.getValue());

			msg_pos.currentTime = start.toString();
			msg_pos.vehicleId = person->client_id;
			msg_pos.latitude = person->xPos.get();
			msg_pos.longtitude = person->yPos.get();

			std::string msg = msg_pos.buildToString();
			msgs.push(msg);
		}
	}

	return msgs;
}

MessageList FMOD_Controller::collectLinkTravelTime()
{
	MessageList msgs;

	for(std::vector<Agent*>::iterator it=allChildren.begin(); it!=allChildren.end(); it++){

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

	MsgLinkTravel msgTravel;
	msgTravel.currentTime = start.toString();
	msgTravel.messageID_ = FMOD_Message::MSG_LINKTRAVELUPADTE;
	for(std::map<const sim_mob::Link*, travelTimes>::iterator itTT=LinkTravelTimesMap.begin(); itTT!=LinkTravelTimesMap.end(); itTT++){
		MsgLinkTravel::Link travel;
		(itTT->first)->getStart()->getID();
		travel.node1Id = (itTT->first)->getStart()->getID();
		travel.node2Id = (itTT->first)->getEnd()->getID();
		travel.wayId = (itTT->first)->getLinkId();
		travel.travelTime = (itTT->second).linkTravelTime;
		msgTravel.links.push_back(travel);
	}

	std::string msg = msgTravel.buildToString();
	msgs.push(msg);

	return msgs;
}

MessageList FMOD_Controller::generateRequest(timeslice now)
{
	MessageList msgs;

	unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();
	DailyTime curr(curTickMS);
	DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());
	typedef std::map<Request*, TripChainItem*>::iterator RequestMap;
	for (RequestMap it=allRequests.begin(); it!=allRequests.end(); it++) {

		DailyTime tm(it->first->departureTimeEarly);
		tm.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime());
		DailyTime dias(1*3600*1000);

		if( tm.getValue() > (curr.getValue()-dias.getValue() )){
			MsgRequest request;
			request.messageID_ = FMOD_Message::MSG_REQUEST;
			request.request = *it->first;
			msgs.push( request.buildToString() );
		}
	}

	return msgs;
}
MessageList FMOD_Controller::handleOfferMessage(const std::string& msg)
{
	MessageList msgs;

	MsgOffer msgOffer;
	msgOffer.createMessage(msg);

	unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();
	DailyTime curr(curTickMS);
	DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());

	MsgAccept msgAccept;
	msgAccept.messageID_ = FMOD_Message::MSG_ACCEPT;
	msgAccept.clientId = msgOffer.clientId;
	msgAccept.scheduleId = msgOffer.offers[0].schduleId;
	msgAccept.arrivalTime = msgOffer.offers[0].arivalTimeEarly;
	msgAccept.departureTime = msgOffer.offers[0].departureTimeEarly;
	msgAccept.currentTime = (curr+base).toString();
	std::string str = msgAccept.buildToString();

	msgs.push( msgAccept.buildToString() );

	return msgs;
}
MessageList FMOD_Controller::handleConfirmMessage(const std::string& msg)
{
	MessageList msgs;

	MsgConfirmation msgConfirm;
	msgConfirm.createMessage(msg);

	FMOD_Message msgFetch;
	msgFetch.messageID_ = FMOD_Message::MSG_SCHEDULE_FETCH;
	std::string str = msgFetch.buildToString();

	msgs.push(str);

	return msgs;
}

void FMOD_Controller::handleScheduleMessage(const std::string& msg)
{
	MsgSchedule msgRequest;
	msgRequest.createMessage( msg );

	FMODSchedule* schedule = new FMODSchedule();
	sim_mob::Node* startNode=nullptr;
	sim_mob::Node* endNode=nullptr;
	for(std::vector<MsgSchedule::Route>::iterator it=msgRequest.routes.begin(); it!=msgRequest.routes.end(); it++){
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
	for(std::vector<MsgSchedule::Stop>::iterator it=msgRequest.stopSchdules.begin(); it!=msgRequest.stopSchdules.end(); it++){
		FMODSchedule::STOP stop;
		stop.stopId = boost::lexical_cast<int>( (*it).stopId );
		stop.scheduleId = boost::lexical_cast<int>( msgRequest.scheduleId );
		stop.dwellTime = 0;
		for(std::vector<std::string>::iterator itP=(*it).alightingPassengers.begin(); itP!=(*it).alightingPassengers.end(); itP++){
			stop.alightingPassengers.push_back( boost::lexical_cast<int>( (*itP) ) );
		}
		for(std::vector<std::string>::iterator itP=(*it).boardingPassengers.begin(); itP!=(*it).boardingPassengers.end(); itP++){
			stop.boardingPassengers.push_back( boost::lexical_cast<int>( (*itP) ) );
		}
		schedule->stopSchdules.push_back(stop);
		if(start.getValue()==0){
			start = DailyTime( (*it).arrivalTime );
		}
	}

	sim_mob::TripChainItem* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", startNode, "node", endNode, "node");
	SubTrip subTrip("-1", "Trip", 0, -1, start, DailyTime(), startNode, "node", endNode, "node", "Car");
	subTrip.schedule = schedule;
	((Trip*)tc)->addSubTrip(subTrip);

	std::vector<sim_mob::TripChainItem*>  tcs;
	tcs.push_back(tc);

	if( parkingCoord.remove(msgRequest.vehicleId) ){
		sim_mob::Person* person = new sim_mob::Person("FMOD_TripChain", ConfigManager::GetInstance().FullConfig().mutexStategy(), tcs);
		person->client_id = msgRequest.vehicleId ;
		person->parentEntity = this;
		allDrivers.push_back(person);
		parkingCoord.enterTo(person->originNode.node_, person);
	}

}

void FMOD_Controller::unregisteredChild(Entity* child)
{
	Agent* agent = dynamic_cast<Agent*>(child);
	if(agent)
	{
		std::vector<Agent*>::iterator it = std::find(allChildren.begin(), allChildren.end(), agent);
		if (it != allChildren.end() ) {
			allChildren.erase(it);

			sim_mob::Person* person = dynamic_cast<Person*>(agent);
			parkingCoord.enterTo(person->originNode.node_, person);
		}
	}
}

void FMOD_Controller::handleVehicleInit(const std::string& msg)
{
	MsgVehicleInit msgInit;
	msgInit.createMessage(msg);

	for(std::vector<MsgVehicleInit::Supply>::iterator it=msgInit.vehicles.begin(); it!=msgInit.vehicles.end(); it++){

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

void FMOD_Controller::dispatchPendingAgents(timeslice now)
{
	unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();
	std::vector<Agent*>::iterator it=allPersons.begin();
	while(it!=allPersons.end()){
		if( (*it)->getStartTime() < curTickMS ){
			this->currWorkerProvider->scheduleForBred( (*it) );
			it = allPersons.erase(it);
		}
		else{
			it++;
		}
	}

	it = allDrivers.begin();
	while(it!=allDrivers.end()){
		if( (*it)->getStartTime() < curTickMS ){
			this->currWorkerProvider->scheduleForBred( (*it) );
			allChildren.push_back((*it));
			it = allDrivers.erase(it);
		}
		else{
			++it;
		}
	}
}

}

} /* namespace sim_mob */
