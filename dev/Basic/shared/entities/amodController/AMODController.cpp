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

bool AMODController::connectAmodService()
{
	bool ret = connectPoint->connectToServer(ipAddress, port);

	if(ret)	{

		boost::thread bt( boost::bind(&boost::asio::io_service::run, &ioService) );

		// for initialization
		sim_mob::FMOD::MsgInitialize request;
		request.messageID_ = sim_mob::FMOD::FMOD_Message::MSG_INITIALIZE;
		request.mapType = "osm";
		request.mapFile = mapFile;
		request.version = 1;
		request.startTime = ConfigManager::GetInstance().FullConfig().simStartTime().toString();
		std::string msg = request.buildToString();
		connectPoint->sendMessage(msg);
		connectPoint->flush();

		std::string message;
		if( !connectPoint->waitMessageInBlocking(message, waitingseconds)){
			std::cout << "AMOD communication in blocking mode not receive data asap" << std::endl;
		}
		else{
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
	std::cout<<"msg"<<std::endl;
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
	unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();


//	if(frameTicks%2 == 0){
//		processMessagesInBlocking(now);
//	}
//	else if(frameTicks%2 == 1){
//		updateMessagesInBlocking(now);
//	}
//
//	dispatchPendingAgents(now);

	return Entity::UpdateStatus::Continue;
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
