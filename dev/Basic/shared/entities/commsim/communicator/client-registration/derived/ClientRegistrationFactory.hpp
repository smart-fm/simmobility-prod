/*
 * ClientRegistrationFactory.hpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */
#include<map>
#include <iostream>
#include <boost/shared_ptr.hpp>

#ifndef CLIENTREGISTRATIONFACTORY_HPP_
#define CLIENTREGISTRATIONFACTORY_HPP_
#include "entities/commsim/communicator/external/base/ClinetRegistrationHandler.hpp"
#include "entities/commsim/communicator/external/derived/android/AndroidClientRegistration.hpp"
#include "entities/commsim/communicator/external/derived/ns3/NS3ClientRegistration.hpp"

namespace sim_mob {

enum ClientType
{
	ANDROID_EMULATOR = 1,
	NS3_SIMULATOR = 2,
	//add your client type here
};
class ClientRegistrationFactory {
	std::map<std::string, ClientType> ClientTypeMap; //a map for ... for... for easy mapping between string and enum
	//This map is used as a cache to avoid repetitive handler creation in heap
	std::map<ClientType, boost::shared_ptr<sim_mob::ClientRegistrationHandler> > ClientRegistrationHandlerMap;
public:
	ClientRegistrationFactory();
	boost::shared_ptr<sim_mob::ClientRegistrationHandler> getHandler(ClientType type);
	virtual ~ClientRegistrationFactory();
};

} /* namespace sim_mob */
#endif /* CLIENTREGISTRATIONFACTORY_HPP_ */
