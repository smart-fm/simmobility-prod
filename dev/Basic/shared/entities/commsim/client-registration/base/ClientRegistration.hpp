/*
 * ClientRegistration.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#ifndef CLIENTREGISTRATION_HPP_
#define CLIENTREGISTRATION_HPP_
#include <set>
#include<map>
#include "entities/commsim/service/services.hpp"
#include <boost/shared_ptr.hpp>
namespace sim_mob
{
//Forward declaration
class Session;
class ClientRegistrationRequest
{
public:
	std::string clientID;
	std::string client_type; //ns3, android emulator, FMOD etc
	std::set<sim_mob::SIM_MOB_SERVICE> requiredServices;
	boost::shared_ptr<Session> session_;
	ClientRegistrationRequest(const ClientRegistrationRequest& other);
	ClientRegistrationRequest();
	ClientRegistrationRequest & operator=(const ClientRegistrationRequest & rhs);

};

typedef std::multimap<std::string,ClientRegistrationRequest > ClientWaitList; //<client type,registrationrequestform >

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

class Broker;

class ClientRegistrationHandler {
public:
	ClientRegistrationHandler();
	virtual bool handle(sim_mob::Broker&, sim_mob::ClientRegistrationRequest) = 0;
	virtual ~ClientRegistrationHandler();
};

}//namespace sim_mob

#endif /* CLIENTREGISTRATION_HPP_ */
