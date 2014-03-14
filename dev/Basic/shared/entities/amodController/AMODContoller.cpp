//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * AMODContoller.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: Max
 */

#include "AMODContoller.hpp"
#include "entities/fmodController/FMOD_Message.hpp"
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
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
sim_mob::AMOD::AMODContoller::AMODContoller() {
	// TODO Auto-generated constructor stub

}

sim_mob::AMOD::AMODContoller::~AMODContoller() {
	// TODO Auto-generated destructor stub
}

bool AMODContoller::connectAmodService()
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
			std::cout << "FMOD communication in blocking mode not receive data asap" << std::endl;
		}
		else{
//			handleVehicleInit(message);
			isConnectFmodServer = true;
		}
	}
	else {
		std::cout << "FMOD communication failed" << std::endl;
		isConnectFmodServer = false;
	}

	return ret;
}

} /* namespace AMOD */
} /* namespace sim_mob */
