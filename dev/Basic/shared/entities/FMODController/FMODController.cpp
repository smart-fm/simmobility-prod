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

	DispatchActivityAgents(now);

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
		tc->startTime = DailyTime(it->first->departure_time_early); //+DailyTime(tc->requestTime*60*1000/2.0);

		std::vector<sim_mob::TripChainItem*>  tcs;
		tcs.push_back(tc);

		sim_mob::Person* person = new sim_mob::Person("FMOD_TripChain", ConfigParams::GetInstance().mutexStategy, tcs);
		person->client_id = it->first->client_id;
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
				request->departure_time_early = DailyTime(cur.getValue()-tc->requestTime*60*1000/2).toString();;
				request->departure_time_late = DailyTime(cur.getValue()+tc->requestTime*60*1000/2).toString();;

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


bool FMODController::StartClientService()
{
	bool ret = false;
	ret = connectPoint->ConnectToServer(ipAddress, port);

	if(ret)	{
		boost::thread bt( boost::bind(&boost::asio::io_service::run, &io_service) );
		std::cout << "FMOD communication success" << std::endl;

		// for initialization
		Msg_Initialize request;
		request.messageID_ = Message::MSG_INITIALIZE;
		request.map_type = "osm";
		request.map_file = mapFile; //"cityhall/cityhall.osm";
		request.version = 1;
		request.start_time = ConfigParams::GetInstance().simStartTime.toString();
		std::string msg = request.BuildToString();
		std::cout << "FMOD Controller send message :" << msg << std::endl;
		connectPoint->pushMessage(msg);
		//connectPoint->pushMessage(msg);
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
		if(msgId == Message::MSG_SIMULATION_SETTINGS ){
			HandleVehicleInit(str);
		}
		else if(msgId == Message::MSG_OFFER ){
			MessageList ret = HandleOfferMessage(str);
			connectPoint->pushMessage(ret);
			continued = true;
		}
		else if(msgId == Message::MSG_CONFIRM ){
			MessageList ret = HandleConfirmMessage(str);
			connectPoint->pushMessage(ret);
			continued = true;
		}
		else if(msgId == Message::MSG_SCHEDULE ){
			HandleScheduleMessage(str);
		}

		if(continued)
			messages = connectPoint->popMessage();
	}

	connectPoint->Flush();
}

void FMODController::UpdateMessages()
{
	MessageList ret;

	ret = CollectVehStops();
	connectPoint->pushMessage(ret);
	ret = CollectVehPos();
	connectPoint->pushMessage(ret);
	ret = CollectLinkTravelTime();
	connectPoint->pushMessage(ret);

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

							Msg_Vehicle_Stop msg_stop;
							msg_stop.messageID_ = Message::MSG_VEHICLESTOP;

							unsigned int curTickMS = (frameTicks)*ConfigParams::GetInstance().baseGranMS;
							DailyTime curr(curTickMS);
							DailyTime base(ConfigParams::GetInstance().simStartTime);
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
		tm.offsetMS_From(ConfigParams::GetInstance().simStartTime);
		DailyTime dias(1*3600*1000);

		if( tm.getValue() > (curr.getValue()-dias.getValue() )){
			Msg_Request request;
			request.messageID_ = Message::MSG_REQUEST;
			request.request = *it->first;
			msgs.push( request.BuildToString() );
		}
	}

	return msgs;
}
MessageList FMODController::HandleOfferMessage(std::string msg)
{
	MessageList msgs;

	Msg_Offer msg_offer;
	msg_offer.CreateMessage(msg);

	unsigned int curTickMS = (frameTicks)*ConfigParams::GetInstance().baseGranMS;
	DailyTime curr(curTickMS);
	DailyTime base(ConfigParams::GetInstance().simStartTime);

	Msg_Accept msg_accept;
	msg_accept.messageID_ = Message::MSG_ACCEPT;
	msg_accept.client_id = msg_offer.client_id;
	msg_accept.schedule_id = msg_offer.offers[0].schdule_id;
	msg_accept.arrival_time = msg_offer.offers[0].arival_time_early;
	msg_accept.departure_time = msg_offer.offers[0].departure_time_early;
	msg_accept.current_time = (curr+base).toString();
	std::string str = msg_accept.BuildToString();

	msgs.push( msg_accept.BuildToString() );

	return msgs;
}
MessageList FMODController::HandleConfirmMessage(std::string msg)
{
	MessageList msgs;

	Msg_Confirmation msg_confirm;
	msg_confirm.CreateMessage(msg);

	return msgs;
}

void FMODController::HandleScheduleMessage(std::string msg)
{
	Msg_Schedule msg_request;
	msg_request.CreateMessage( msg );

	FMODSchedule* schedule = new FMODSchedule();
	sim_mob::Node* startNode=nullptr;
	sim_mob::Node* endNode=nullptr;
	for(std::vector<Msg_Schedule::ROUTE>::iterator it=msg_request.routes.begin(); it!=msg_request.routes.end(); it++){
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
	for(std::vector<Msg_Schedule::STOP>::iterator it=msg_request.stop_schdules.begin(); it!=msg_request.stop_schdules.end(); it++){
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

	sim_mob::Person* person = new sim_mob::Person("FMOD_TripChain", ConfigParams::GetInstance().mutexStategy, tcs);
	person->client_id = boost::lexical_cast<int>( msg_request.vehicle_id );
	person->parentEntity = this;
	all_drivers.push_back(person);
}

void FMODController::unregisteredChild(Entity* child)
{
	Agent* agent = dynamic_cast<Agent*>(child);
	if(agent)
	{
		std::vector<Agent*>::iterator it = std::find(all_children.begin(), all_children.end(), agent);
		if (it != all_children.end() ) {
			all_children.erase(it);
		}
	}
}

void FMODController::HandleVehicleInit(std::string msg)
{
	msgVehInit.CreateMessage(msg);
}

void FMODController::DispatchActivityAgents(timeslice now)
{
	unsigned int curTickMS = (frameTicks)*ConfigParams::GetInstance().baseGranMS;
	std::vector<Agent*>::iterator it=all_persons.begin();
	while(it!=all_persons.end()){
		if( (*it)->getStartTime() < curTickMS ){
			currWorker->scheduleForBred( (*it) );
			it = all_persons.erase(it);
		}
		else{
			it++;
		}
	}

	it = all_drivers.begin();
	while(it!=all_drivers.end()){
		if( (*it)->getStartTime() < curTickMS ){
			currWorker->scheduleForBred( (*it) );
			all_children.push_back((*it));
			it = all_drivers.erase(it);
		}
		else{
			++it;
		}
	}

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

		/*Activity* waiting = new Activity();
		waiting->personID = 1;
		waiting->itemType = TripChainItem::IT_ACTIVITY;
		waiting->sequenceNumber = 1;
		waiting->description = "waiting";
		waiting->isPrimary = false;
		waiting->isFlexible = false;
		waiting->isMandatory = false;
		waiting->location = node5;
		waiting->locationType = TripChainItem::LT_NODE;
		waiting->startTime = start;
		waiting->endTime = DailyTime(10*60*1000)+ConfigParams::GetInstance().simStartTime;;
		tcs.push_back(waiting);*/

		sim_mob::Person* person = new sim_mob::Person("FMOD_TripChain", ConfigParams::GetInstance().mutexStategy, tcs);
		person->parentEntity = this;
		person->client_id = 101;
		all_drivers.push_back(person);
	}


}

}

} /* namespace sim_mob */
