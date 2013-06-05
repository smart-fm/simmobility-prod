/*
 * ClientRegistrationFactory.hpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */
#ifndef CLIENTREGISTRATIONFACTORY_HPP_
#define CLIENTREGISTRATIONFACTORY_HPP_
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
//	std::map<std::string, ClientType> ClientTypeMap; //a map for ... for... for easy mapping between string and enum
	//This map is used as a cache to avoid repetitive handler creation in heap
	std::map<sim_mob::ClientType, boost::shared_ptr<sim_mob::ClientRegistrationHandler> > ClientRegistrationHandlerMap;
public:
	ClientRegistrationFactory();
	boost::shared_ptr<sim_mob::ClientRegistrationHandler> getHandler(ClientType type);
	virtual ~ClientRegistrationFactory();
};

} /* namespace sim_mob */
#endif /* CLIENTREGISTRATIONFACTORY_HPP_ */
