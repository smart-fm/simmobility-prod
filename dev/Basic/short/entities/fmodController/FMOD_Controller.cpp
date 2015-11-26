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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <stdexcept>
#include <utility>

#include "FMOD_Controller.hpp"
#include "entities/Person_ST.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/network/RoadNetwork.hpp"

namespace sim_mob
{

namespace FMOD
{

FMOD_Controller* FMOD_Controller::pInstance = nullptr;
boost::asio::io_service FMOD_Controller::ioService;
const std::string FMOD_Controller::FMOD_Trip = "FMOD_TripChain";

void FMOD_Controller::registerController(int id, const MutexStrategy& mtxStrat)
{
	if (pInstance)
	{
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
	if (!pInstance)
	{
		throw std::runtime_error("Fmod controller instance is null.");
	}

	return pInstance;
}

FMOD_Controller::~FMOD_Controller()
{
	// TODO Auto-generated destructor stub
	stopClientService();
}

Entity::UpdateStatus FMOD_Controller::frame_init(timeslice now)
{
	initialize();
	bool ret = connectFmodService();
	return ret ? Entity::UpdateStatus::Continue : Entity::UpdateStatus::Done;
}

Entity::UpdateStatus FMOD_Controller::frame_tick(timeslice now)
{
	if (!isConnectFmodServer)
		return Entity::UpdateStatus::Continue;

	processMessagesInBlocking(now);

	updateMessagesInBlocking(now);

	dispatchPendingAgents(now);

	return Entity::UpdateStatus::Continue;
}

void FMOD_Controller::frame_output(timeslice now)
{

}

void FMOD_Controller::finalizeMessageToFMOD()
{
	if (!isConnectFmodServer)
		return;

	MsgFinalize request;
	request.messageID_ = FMOD_Message::MSG_FINALIZE;

	DailyTime endTime(ConfigManager::GetInstance().FullConfig().simStartTime().getValue() + ConfigManager::GetInstance().FullConfig().totalRuntimeInMilliSeconds());
	request.end_time = endTime.getStrRepr();
	std::string msg = request.buildToString();
	connectPoint->sendMessage(msg);
	connectPoint->flush();

	std::string message;
	if (!connectPoint->waitMessageInBlocking(message, waitingSeconds))
	{
		std::cout << "FMOD communication in blocking mode not receive data asap" << std::endl;
	}
}

std::string FMOD_Controller::getFOMD_Time(std::string ts)
{
	boost::posix_time::ptime pt(boost::posix_time::time_from_string(ts));
	boost::posix_time::time_facet* tf = new boost::posix_time::time_facet("%H:%M:%S");
	std::stringstream ss;
	ss.imbue(std::locale(std::cout.getloc(), tf));

	return ss.str();
}

bool FMOD_Controller::insertFmodItems(const std::string& personID, TripChainItem* item)
{
	allItems[personID] = item;
	return true;
}

void FMOD_Controller::collectPerson()
{
	std::map<Request*, TripChainItem*>::iterator it;
	for (it = allRequests.begin(); it != allRequests.end(); it++)
	{

		sim_mob::TripChainItem* tc = it->second;
		tc->setPersonID(boost::lexical_cast<std::string>(it->first->clientId));
		tc->startTime = DailyTime(it->first->departureTimeEarly);

		std::vector<sim_mob::TripChainItem*> tcs;
		tcs.push_back(tc);

		Trip* trip = dynamic_cast<Trip*> (it->second);
		Person_ST* person = new Person_ST(FMOD_Controller::FMOD_Trip, ConfigManager::GetInstance().FullConfig().mutexStategy(), tcs);
		person->client_id = it->first->clientId;
		person->yPos.force(trip->origin.node->getLocation().getY());
		person->xPos.force(trip->origin.node->getLocation().getX());
		allPersons.push_back(person);
	}
}

void FMOD_Controller::collectRequest()
{
	std::map<std::string, TripChainItem*>::iterator it;
	for (it = allItems.begin(); it != allItems.end(); it++)
	{

		Print() << "request person id " << it->first << std::endl;
		Trip* trip = dynamic_cast<Trip*> (it->second);

		if (trip == nullptr)
		{
			continue;
		}

		const RoadNetwork *network = RoadNetwork::getInstance();
		const Node* node = network->getById(network->getMapOfIdvsNodes(), boost::lexical_cast<unsigned int>(trip->startLocationId));
		trip->origin = WayPoint(node);
		node = network->getById(network->getMapOfIdvsNodes(), boost::lexical_cast<unsigned int>(trip->endLocationId));
		trip->destination = WayPoint(node);

		if (trip->sequenceNumber > 0)
		{

			float period = trip->endTime.getValue() - trip->startTime.getValue();
			period = period / trip->sequenceNumber;
			DailyTime tm(period);
			DailyTime current = trip->startTime;
			const unsigned int MIN_MS = 60000;
			DailyTime bias = DailyTime(trip->requestTime * MIN_MS);
			int id = boost::lexical_cast<int>(it->first);

			for (int i = 0; i < trip->sequenceNumber; i++)
			{
				Request* request = new Request();
				request->clientId = id + i;
				request->arrivalTimeEarly = "-1";
				request->arrivalTimeLate = "-1";
				request->seatNum = 1;

				if (trip->origin.type == WayPoint::NODE)
				{
					request->originLongitude = boost::lexical_cast<std::string>(trip->origin.node->getLocation().getX());
					request->originLatitude = boost::lexical_cast<std::string>(trip->origin.node->getLocation().getY());
				}
				if (trip->destination.type == WayPoint::NODE)
				{
					request->destLongitude = boost::lexical_cast<std::string>(trip->destination.node->getLocation().getX());
					request->destLatitude = boost::lexical_cast<std::string>(trip->destination.node->getLocation().getY());
				}

				current += tm;
				unsigned int requestWindow = trip->requestTime * MIN_MS / 2;
				request->departureTimeEarly = DailyTime(current.getValue() - requestWindow).getStrRepr();
				request->departureTimeLate = DailyTime(current.getValue() + requestWindow).getStrRepr();
				allRequests.insert(std::make_pair(request, trip));
			}
		}
		else
		{
			allItems[it->first] = nullptr;
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

	if (ret)
	{

		boost::thread bt(boost::bind(&boost::asio::io_service::run, &ioService));

		// for initialization
		MsgInitialize request;
		request.messageID_ = FMOD_Message::MSG_INITIALIZE;
		request.mapType = "osm";
		request.mapFile = mapFile;
		request.version = 1;
		request.startTime = ConfigManager::GetInstance().FullConfig().simStartTime().getStrRepr();
		std::string msg = request.buildToString();
		connectPoint->sendMessage(msg);
		connectPoint->flush();

		std::string message;
		if (!connectPoint->waitMessageInBlocking(message, waitingSeconds))
		{
			std::cout << "FMOD communication in blocking mode not receive data asap" << std::endl;
		}
		else
		{
			handleVehicleInit(message);
			isConnectFmodServer = true;
			publisher.registerEvent(EVENT_DISPATCH_FMOD_SCHEDULES_REQUEST);
		}
	}
	else
	{
		std::cout << "FMOD communication failed" << std::endl;
		isConnectFmodServer = false;
	}

	return ret;
}

void FMOD_Controller::stopClientService()
{
	if (!isConnectFmodServer)
		return;

	connectPoint->stop();
	ioService.stop();
	isConnectFmodServer = false;
}

void FMOD_Controller::processMessagesInBlocking(timeslice now)
{
	if (!isConnectFmodServer)
		return;

	MessageList requests = generateRequest(now);

	while (requests.size() > 0)
	{
		std::string message = requests.front();
		connectPoint->sendMessage(message);
		requests.pop();

		while (true)
		{

			connectPoint->flush();
			if (!connectPoint->waitMessageInBlocking(message, waitingSeconds))
			{
				std::cout << "FMOD communication in blocking mode not receive data asap" << std::endl;
				break;
			}

			int msgId = FMOD_Message::analyzeMessageID(message);
			if (msgId == FMOD_Message::MSG_OFFER)
			{
				MessageList ret = handleOfferMessage(now, message);
				connectPoint->sendMessage(ret);
				continue;
			}
			else if (msgId == FMOD_Message::MSG_CONFIRMATION)
			{
				MessageList ret = handleConfirmMessage(message);
				connectPoint->sendMessage(ret);
				continue;
			}
			else if (msgId == FMOD_Message::MSG_SCHEDULE)
			{
				handleScheduleMessage(message);
				break;
			}
		}
	}
}

void FMOD_Controller::processMessages(timeslice now)
{
	if (!isConnectFmodServer)
		return;

	MessageList Requests = generateRequest(now);
	connectPoint->sendMessage(Requests);
	connectPoint->flush();

	bool continued = false;
	MessageList messages = connectPoint->getMessage();
	while (messages.size() > 0)
	{
		std::string str = messages.front();
		messages.pop();

		int msgId = FMOD_Message::analyzeMessageID(str);
		if (msgId == FMOD_Message::MSG_SIMULATION_SETTINGS)
		{
			handleVehicleInit(str);
		}
		else if (msgId == FMOD_Message::MSG_OFFER)
		{
			MessageList ret = handleOfferMessage(now, str);
			connectPoint->sendMessage(ret);
			continued = true;
		}
		else if (msgId == FMOD_Message::MSG_CONFIRMATION)
		{
			MessageList ret = handleConfirmMessage(str);
			connectPoint->sendMessage(ret);
			continued = true;
		}
		else if (msgId == FMOD_Message::MSG_SCHEDULE)
		{
			handleScheduleMessage(str);
		}

		if (continued)
			messages = connectPoint->getMessage();
	}

	connectPoint->flush();
}

void FMOD_Controller::updateMessagesInBlocking(timeslice now)
{
	if (!isConnectFmodServer)
		return;

	std::string message;
	MessageList ret;
	ret = collectVehStops(now);
	if (ret.size() > 0)
	{
		connectPoint->sendMessage(ret);
		connectPoint->flush();
		connectPoint->waitMessageInBlocking(message, waitingSeconds);
		if (FMOD_Message::analyzeMessageID(message) != FMOD_Message::MSG_ACK)
		{
			std::cout << "Fmod Controller not receive correct acknowledge message" << std::endl;
			return;
		}
	}

	if (now.ms() % updateTime == 0)
	{
		ret = collectVehPos(now);
		if (ret.size() > 0)
		{
			connectPoint->sendMessage(ret);
			connectPoint->flush();
			connectPoint->waitMessageInBlocking(message, waitingSeconds);
			if (FMOD_Message::analyzeMessageID(message) != FMOD_Message::MSG_ACK)
			{
				std::cout << "Fmod Controller not receive correct acknowledge message" << std::endl;
				return;
			}
		}
	}

	if (now.ms() % updateTime == 0)
	{
		ret = collectLinkTravelTime(now);
		if (ret.size() > 0)
		{
			connectPoint->sendMessage(ret);
			connectPoint->flush();
			connectPoint->waitMessageInBlocking(message, waitingSeconds);
			if (FMOD_Message::analyzeMessageID(message) != FMOD_Message::MSG_ACK)
			{
				std::cout << "Fmod Controller not receive correct acknowledge message" << std::endl;
				return;
			}
		}
	}
}

void FMOD_Controller::updateMessages(timeslice now)
{
	if (!isConnectFmodServer)
		return;

	MessageList ret = collectVehStops(now);
	connectPoint->sendMessage(ret);

	if (now.ms() % updateTime == 0)
	{
		ret = collectVehPos(now);
		connectPoint->sendMessage(ret);
	}

	if (now.ms() % updateTime == 0)
	{
		ret = collectLinkTravelTime(now);
		connectPoint->sendMessage(ret);
	}

	connectPoint->flush();
}

MessageList FMOD_Controller::collectVehStops(timeslice now)
{
	MessageList msgs;
	for (std::vector<Person_ST*>::iterator it = allChildren.begin(); it != allChildren.end(); it++)
	{
		Person_ST* person = (*it);
		if (person)
		{

			int client_id = person->client_id;
			Role<Person_ST>* driver = person->getRole();

			if (driver)
			{
				std::vector<sim_mob::BufferedBase*> res = driver->getDriverInternalParams();
				if (res.size() > 0)
				{
					Shared<std::string>* stopEvtTime = dynamic_cast<Shared<std::string>*> (res[0]);
					Shared<int>* stopEvtType = dynamic_cast<Shared<int>*> (res[1]);
					Shared<int>* stopEvtScheduleId = dynamic_cast<Shared<int>*> (res[2]);
					Shared<int>* stopEvtNodeId = dynamic_cast<Shared<int>*> (res[3]);
					Shared< std::vector<int> >* stopEvtBoardingPassengers = dynamic_cast<Shared< std::vector<int> >*> (res[4]);
					Shared< std::vector<int> >* stopEvtAlightingPassengers = dynamic_cast<Shared< std::vector<int> >*> (res[5]);

					if (stopEvtTime && stopEvtType && stopEvtScheduleId && stopEvtBoardingPassengers && stopEvtAlightingPassengers)
					{
						if (stopEvtType->get() > 0)
						{

							DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());
							const int Second_MS = 1000;
							DailyTime current(((now.ms() + base.getValue()) / Second_MS) * Second_MS);

							if (stopEvtType->get() != EVENT_BYPASS_IMMEDIATESTOP)
							{
								MsgVehicleStop msgStop;
								msgStop.messageID_ = FMOD_Message::MSG_VEHICLESTOP;
								msgStop.currentTime = current.getStrRepr();
								msgStop.eventType = stopEvtType->get() - EVENT_DEPARTURE_STARTSTOP;
								msgStop.scheduleId = boost::lexical_cast<std::string>(stopEvtScheduleId->get());
								msgStop.stopId = boost::lexical_cast<std::string>(stopEvtNodeId->get());
								msgStop.vehicleId = boost::lexical_cast<std::string>(client_id);
								msgStop.boardingPassengers = *(stopEvtBoardingPassengers);
								msgStop.aligtingPassengers = *(stopEvtAlightingPassengers);

								std::string msg = msgStop.buildToString();
								msgs.push(msg);
							}
							else
							{
								MsgVehicleStop msgStopArr;
								msgStopArr.messageID_ = FMOD_Message::MSG_VEHICLESTOP;
								msgStopArr.currentTime = current.getStrRepr();
								msgStopArr.eventType = EVENT_ARRIVAL_IMMEDIATESTOP - EVENT_DEPARTURE_STARTSTOP;
								msgStopArr.scheduleId = boost::lexical_cast<std::string>(stopEvtScheduleId->get());
								msgStopArr.stopId = boost::lexical_cast<std::string>(stopEvtNodeId->get());
								msgStopArr.vehicleId = boost::lexical_cast<std::string>(client_id);
								std::string msg = msgStopArr.buildToString();
								msgs.push(msg);

								msgStopArr.eventType = EVENT_DEPARTURE_IMMEDIATESTOP - EVENT_DEPARTURE_STARTSTOP;
								msg = msgStopArr.buildToString();
								msgs.push(msg);
							}

							stopEvtType->set(EVENT_STOP_DEFALUTEVALUE);
						}
					}
				}
			}
		}
	}
	return msgs;
}

MessageList FMOD_Controller::collectVehPos(timeslice now)
{
	MessageList msgs;

	for (std::vector<Person_ST*>::iterator it = allChildren.begin(); it != allChildren.end(); it++)
	{
		Person_ST* person = (*it);
		if (person)
		{
			MsgVehiclePos msgPos;
			msgPos.messageID_ = FMOD_Message::MSG_VEHICLEPOS;

			DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());
			const int Second_MS = 1000;
			DailyTime current(((now.ms() + base.getValue()) / Second_MS) * Second_MS);

			msgPos.currentTime = current.getStrRepr();
			msgPos.vehicleId = boost::lexical_cast<std::string>(person->client_id);
			msgPos.latitude = person->xPos.get();
			msgPos.longtitude = person->yPos.get();

			std::string msg = msgPos.buildToString();
			msgs.push(msg);
		}
	}

	return msgs;
}

MessageList FMOD_Controller::collectLinkTravelTime(timeslice now)
{
	MessageList msgs;
	bool isEmpty = false;
	for (std::vector<Person_ST*>::iterator it = allChildren.begin(); it != allChildren.end(); it++)
	{
		const std::map<double, LinkTravelStats> travelStatsMap;// = (*it)->linkTravelStatsMap.get();
		for (std::map<double, LinkTravelStats>::const_iterator itTravel = travelStatsMap.begin(); itTravel != travelStatsMap.end(); itTravel++)
		{

			double travelTime = (*it)->currLinkTravelStats.travelTime; // = (itTravel->first) - (itTravel->second).linkEntryTime_;
			std::map<const Link*, travelTimes>::iterator itTT = LinkTravelTimesMap.find((itTravel->second).link);

			if (itTT != LinkTravelTimesMap.end())
			{
				itTT->second.agentCount = itTT->second.agentCount + 1;
				itTT->second.linkTravelTime = itTT->second.linkTravelTime + travelTime;
			}
			else
			{
				travelTimes tTimes(travelTime, 1);
				LinkTravelTimesMap.insert(std::make_pair((itTravel->second).link, tTimes));
			}
		}
	}

	DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());
	const int Second_MS = 1000;
	DailyTime current(((now.ms() + base.getValue()) / Second_MS) * Second_MS);

	MsgLinkTravel msgTravel;
	msgTravel.currentTime = current.getStrRepr();
	msgTravel.messageID_ = FMOD_Message::MSG_LINKTRAVELUPADTE;
	for (std::map<const sim_mob::Link*, travelTimes>::iterator itTT = LinkTravelTimesMap.begin(); itTT != LinkTravelTimesMap.end(); itTT++)
	{
		MsgLinkTravel::Link travel;
		(itTT->first)->getFromNode()->getNodeId();
		travel.node1Id = (itTT->first)->getFromNode()->getNodeId();
		travel.node2Id = (itTT->first)->getToNode()->getNodeId();
		travel.wayId = (itTT->first)->getLinkId();
		travel.travelTime = (itTT->second).linkTravelTime;
		msgTravel.links.push_back(travel);
		isEmpty = true;
	}

	if (isEmpty)
	{
		std::string msg = msgTravel.buildToString();
		msgs.push(msg);
	}

	return msgs;
}

MessageList FMOD_Controller::generateRequest(timeslice now)
{
	MessageList msgs;

	DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());
	const int Second_MS = 1000;
	DailyTime current(((now.ms() + base.getValue()) / Second_MS) * Second_MS);
	typedef std::map<Request*, TripChainItem*>::iterator RequestMap;
	RequestMap itr = allRequests.begin();
	while (itr != allRequests.end())
	{

		DailyTime tm(itr->first->departureTimeEarly);

		if (tm.getValue() <= current.getValue())
		{
			MsgRequest request;
			request.currentTime = current.getStrRepr();
			request.messageID_ = FMOD_Message::MSG_REQUEST;
			request.request = *itr->first;
			msgs.push(request.buildToString());

			allRequests.erase(itr++);
		}
		else
		{
			++itr;
		}
	}

	return msgs;
}

MessageList FMOD_Controller::handleOfferMessage(timeslice now, const std::string& msg)
{
	MessageList msgs;

	MsgOffer msgOffer;
	msgOffer.createMessage(msg);

	DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());
	const int Second_MS = 1000;
	DailyTime current(((now.ms() + base.getValue()) / Second_MS) * Second_MS);

	MsgAccept msgAccept;
	msgAccept.messageID_ = FMOD_Message::MSG_ACCEPT;
	msgAccept.clientId = msgOffer.clientId;
	msgAccept.currentTime = current.getStrRepr();
	msgAccept.requestId = msgOffer.requestId;
	if (msgOffer.offers.size() > 0)
	{
		msgAccept.accept.productId = msgOffer.offers[0].productId;
		msgAccept.accept.pickupTime = msgOffer.offers[0].arivalTimeEarly;
		msgAccept.accept.dropoffTime = msgOffer.offers[0].departureTimeEarly;
	}
	else
	{
		std::cout << "warning:FMOD not provide any offer!" << std::endl;
	}

	std::string str = msgAccept.buildToString();
	msgs.push(msgAccept.buildToString());

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
	msgRequest.createMessage(msg);

	sim_mob::Node* startNode = msgRequest.getStartNode();
	sim_mob::Node* endNode = msgRequest.getEndNode();

	if (!startNode || !endNode)
	{
		std::cout << "warning:FMOD not provide correct Node id!" << std::endl;
		return;
	}

	DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());
	DailyTime start(msgRequest.getStartTime() > base.getValue() ? msgRequest.getStartTime() : base.getValue());

	if (parkingCoord.remove(msgRequest.getVehicleId()))
	{
		sim_mob::Trip* trip = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", startNode, "node", endNode, "node");
		SubTrip subTrip("-1", "Trip", 0, -1, start, DailyTime(), startNode, "node", endNode, "node", "Car");
		trip->addSubTrip(subTrip);
		std::vector<sim_mob::TripChainItem*> tcs;
		tcs.push_back(trip);

		Person_ST* person = new Person_ST("FMOD_TripChain", ConfigManager::GetInstance().FullConfig().mutexStategy(), tcs);
		person->client_id = msgRequest.getVehicleId();
		person->parentEntity = this;
		person->yPos.force(startNode->getLocation().getY());
		person->xPos.force(startNode->getLocation().getX());
		//publisher.subscribe((event::EventId)sim_mob::FMOD::EVENT_DISPATCH_FMOD_SCHEDULES_REQUEST, person);
		allDrivers.push_back(person);
	}
	else
	{
		std::vector<Person_ST*>::iterator it;
		for (it = allChildren.begin(); it != allChildren.end(); it++)
		{
			if ((*it)->client_id == msgRequest.getVehicleId())
			{
				//publisher.publish((event::EventId)sim_mob::FMOD::EVENT_DISPATCH_FMOD_SCHEDULES_REQUEST, (*it), FMOD_RequestEventArgs());
				messaging::MessageBus::PublishEvent(EVENT_DISPATCH_FMOD_SCHEDULES_REQUEST, (*it),
													messaging::MessageBus::EventArgsPtr(new sim_mob::FMOD_RequestEventArgs(msgRequest.schedules)));
			}
		}
	}
}

void FMOD_Controller::unregisteredChild(Entity* child)
{
	Agent* agent = dynamic_cast<Agent*> (child);
	if (agent)
	{
		std::vector<Person_ST*>::iterator it = std::find(allChildren.begin(), allChildren.end(), agent);
		if (it != allChildren.end())
		{
			allChildren.erase(it);

			Person_ST* person = dynamic_cast<Person_ST*> (agent);
			parkingCoord.enterTo(person->originNode.node, person);
		}
	}
}

void FMOD_Controller::handleVehicleInit(const std::string& msg)
{
	MsgVehicleInit msgInit;
	msgInit.createMessage(msg);

	for (std::vector<MsgVehicleInit::Supply>::iterator it = msgInit.vehicles.begin(); it != msgInit.vehicles.end(); it++)
	{
		const StreetDirectory& stdir = StreetDirectory::Instance();
		Node* node = nullptr; //const_cast<Node*>( stdir.getNode( (*it).nodeId ) );
		if (node != nullptr)
		{
			DailyTime start(0);
			sim_mob::Trip* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", node, "node", node, "node");
			sim_mob::SubTrip subTrip("", "Trip", 0, 1, DailyTime(), DailyTime(), node, "node", node, "node", "Car");
			tc->addSubTrip(subTrip);
			std::vector<sim_mob::TripChainItem*> tcs;
			tcs.push_back(tc);

			Person_ST* person = new Person_ST("FMOD_TripChain", ConfigManager::GetInstance().FullConfig().mutexStategy(), tcs);
			person->parentEntity = this;
			person->client_id = (*it).vehicleId;

			parkingCoord.enterTo(node, person);
		}
	}

}

void FMOD_Controller::dispatchPendingAgents(timeslice now)
{
	DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());
	DailyTime current(now.ms() + base.getValue());

	std::vector<Person_ST*>::iterator it = allPersons.begin();
	while (it != allPersons.end())
	{
		if ((*it)->getStartTime() < current.getValue())
		{
			this->currWorkerProvider->scheduleForBred((*it));
			it = allPersons.erase(it);
		}
		else
		{
			it++;
		}
	}

	it = allDrivers.begin();
	while (it != allDrivers.end())
	{
		if ((*it)->getStartTime() < current.getValue())
		{
			this->currWorkerProvider->scheduleForBred((*it));
			allChildren.push_back((*it));
			it = allDrivers.erase(it);
		}
		else
		{
			++it;
		}
	}
}

}

} /* namespace sim_mob */
