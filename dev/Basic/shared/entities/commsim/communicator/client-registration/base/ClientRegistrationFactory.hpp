//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ClientRegistrationFactory.hpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */
#pragma once

#include<map>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include "entities/commsim/communicator/service/services.hpp"

namespace sim_mob {
//Forward Declaration
class ClientRegistrationHandler;/*
enum ClientType
{
	ANDROID_EMULATOR = 1,
	NS3_SIMULATOR = 2,
	//add your client type here
};*/
class ClientRegistrationFactory {
//	std::map<std::string, ConfigParams::ClientType> ClientTypeMap; //a map for ... for... for easy mapping between string and enum
	//This map is used as a cache to avoid repetitive handler creation in heap
	std::map<ConfigParams::ClientType, boost::shared_ptr<sim_mob::ClientRegistrationHandler> > ClientRegistrationHandlerMap;
public:
	ClientRegistrationFactory();
	boost::shared_ptr<sim_mob::ClientRegistrationHandler> getHandler(ConfigParams::ClientType type);
	virtual ~ClientRegistrationFactory();
};

} /* namespace sim_mob */
