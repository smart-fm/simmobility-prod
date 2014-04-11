//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * AMODController.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: Max
 */

#include "AMODController.hpp"
#include "entities/fmodController/FMOD_Message.hpp"
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include "entities/Person.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/Link.hpp"
#include <utility>
#include <stdexcept>

namespace sim_mob {
namespace AMOD
{
AMODController* AMODController::pInstance = nullptr;
boost::asio::io_service AMODController::ioService;

sim_mob::AMOD::AMODController::~AMODController() {
	// TODO Auto-generated destructor stub
}
bool AMODController::instanceExists()
{
	return pInstance;
}
bool AMODController::connectAmodService()
{
	bool ret = connectPoint->connectToServer(ipAddress, port);

	if(ret)	{

		boost::thread bt( boost::bind(&boost::asio::io_service::run, &ioService) );

		// for initialization
		sim_mob::FMOD::MsgInitialize request;
		request.messageID_ = sim_mob::FMOD::FMOD_Message::MSG_INITIALIZE;
		request.mapType = "xml";
		request.mapFile = "NetworkCopy_small_bugis.xml";//mapFile;
		request.version = 1;
		request.startTime = ConfigManager::GetInstance().FullConfig().simStartTime().toString();
		std::string msg = request.buildToString();
		std::cout<<"msg: "<<msg<<std::endl;
		connectPoint->sendMessage(msg);
		connectPoint->flush();

		std::string message;
		if( !connectPoint->waitMessageInBlocking(message, waitingseconds)){
			std::cout << "AMOD communication in blocking mode not receive data asap" << std::endl;
		}
		else{
			std::cout<<"message: <"<<message<<">"<<std::endl;
			handleVehicleInit(message);
			isConnectAmodServer = true;
		}
	}
	else {
		std::cout << "AMOD communication failed" << std::endl;
		isConnectAmodServer = false;
	}

	return ret;
}
void AMODController::handleVehicleInit(const std::string& msg)
{
	FMOD::MsgVehicleInit msgInit;
	msgInit.createMessage(msg);

	for(std::vector<FMOD::MsgVehicleInit::Supply>::iterator it=msgInit.vehicles.begin(); it!=msgInit.vehicles.end(); it++){

		const StreetDirectory& stdir = StreetDirectory::instance();
		Node* node = const_cast<Node*>( stdir.getNode( (*it).nodeId ) );
		if(node != nullptr){
			DailyTime start(0);
			sim_mob::Trip* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", node, "node", node, "node");
			sim_mob::SubTrip subTrip("", "Trip", 0, 1, DailyTime(), DailyTime(), node, "node", node, "node", "Car");
			tc->addSubTrip(subTrip);
			std::vector<sim_mob::TripChainItem*>  tcs;
			tcs.push_back(tc);

			sim_mob::Person* person = new sim_mob::Person("FMOD_TripChain", ConfigManager::GetInstance().FullConfig().mutexStategy(), tcs);
			person->parentEntity = this;
			person->client_id = (*it).vehicleId;

			parkingCoord.enterTo(node, person);
		}
	}
}
void AMODController::registerController(int id, const MutexStrategy& mtxStrat)
{
	if(pInstance) {
		delete pInstance;
	}

	pInstance = new AMODController(id, mtxStrat);
}
AMODController* AMODController::instance()
{
	if (!pInstance) {
		throw std::runtime_error("Amod controller instance is null.");
	}

	return pInstance;
}

bool AMODController::frame_init(timeslice now)
{
	return true;
}

Entity::UpdateStatus AMODController::frame_tick(timeslice now)
{
	frameTicks++;

	if(frameTicks%100 == 0){
		updateMessagesInBlocking(now);
	}
	else if(frameTicks%2 == 1){
//		processMessagesInBlocking(now);
	}

//	dispatchPendingAgents(now);

	return Entity::UpdateStatus::Continue;

}
void AMODController::updateMessagesInBlocking(timeslice now)
{
	if(!isConnectAmodServer)
		return;

//	std::string message;
//	FMOD::MessageList ret = collectVehStops();
	//std::string msg = "message ID, 0, {"current_time":"08:00:00 01/31/2013", “link":[{"segment_id":"1", "travel_time":30},{"segment_id":"2", "travel_time":10}]}

	AMODMsgLinkTravelTime ret;

	AMODMsgLinkTravelTime::Link l1; l1.segmentId=10; l1.travelTime = 30.0;
	AMODMsgLinkTravelTime::Link l2; l2.segmentId=20; l2.travelTime = 40.0;
	AMODMsgLinkTravelTime::Link l3; l3.segmentId=30; l3.travelTime = 60.0;

	ret.links.push_back(l1);
	ret.links.push_back(l2);
	ret.links.push_back(l3);

	unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();
		DailyTime curr(curTickMS);
		DailyTime base(ConfigManager::GetInstance().FullConfig().simStartTime());
		DailyTime start(curr.getValue()+base.getValue());

	ret.currentTime = start.toString();

	std::string s=ret.buildToString();
	std::cout<<"s: "<<s<<std::endl;
		connectPoint->sendMessage(s);
		connectPoint->flush();
//		connectPoint->waitMessageInBlocking(message, waitingseconds);
//		if(FMOD_Message::analyzeMessageID(message)!= FMOD_Message::MSG_ACK ){
//			std::cout << "Fmod Controller not receive correct acknowledge message" << std::endl;
//			return;
//		}
//	}
//
//	if(now.ms()%updatePosTime == 0){
//		ret = collectVehPos();
//		if(ret.size()>0){
//			connectPoint->sendMessage(ret);
//			connectPoint->flush();
//			connectPoint->waitMessageInBlocking(message, waitingseconds);
//			if(FMOD_Message::analyzeMessageID(message)!= FMOD_Message::MSG_ACK ){
//				std::cout << "Fmod Controller not receive correct acknowledge message" << std::endl;
//				return;
//			}
//		}
//	}
//
//	if(now.ms()%updateTravelTime == 0){
//		ret = collectLinkTravelTime();
//		if(ret.size()>0){
//			connectPoint->sendMessage(ret);
//			connectPoint->flush();
//			connectPoint->waitMessageInBlocking(message, waitingseconds);
//			if(FMOD_Message::analyzeMessageID(message)!= FMOD_Message::MSG_ACK ){
//				std::cout << "Fmod Controller not receive correct acknowledge message" << std::endl;
//				return;
//			}
//		}
//	}
}
void AMODController::frame_output(timeslice now)
{

}
//void AMODController::unregisteredChild(Entity* child)
//{
//	Agent* agent = dynamic_cast<Agent*>(child);
//	if(agent)
//	{
//		std::vector<Agent*>::iterator it = std::find(allChildren.begin(), allChildren.end(), agent);
//		if (it != allChildren.end() ) {
//			allChildren.erase(it);
//
//			sim_mob::Person* person = dynamic_cast<Person*>(agent);
//			parkingCoord.enterTo(person->originNode.node_, person);
//		}
//	}
//}




} /* namespace AMOD */
} /* namespace sim_mob */
