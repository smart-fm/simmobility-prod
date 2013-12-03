//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ClientRegistration.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#pragma once

#include <set>
#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include "entities/commsim/service/Services.hpp"
#include "entities/commsim/connection/Session.hpp"
#include "event/EventPublisher.hpp"

namespace sim_mob {

/******************************************************************************************************
 ***********************************ClientRegistrationRequest****************************************
 ******************************************************************************************************
 */
class ClientRegistrationRequest {
public:
	std::string clientID;
	std::string client_type; //ns3, android emulator, FMOD etc
	std::set<sim_mob::Services::SIM_MOB_SERVICE> requiredServices;
	session_ptr session_;
	ClientRegistrationRequest(const ClientRegistrationRequest& other);
	ClientRegistrationRequest();
	ClientRegistrationRequest & operator=(const ClientRegistrationRequest & rhs);

};

typedef std::multimap<std::string,ClientRegistrationRequest > ClientWaitList; //<client type,registrationrequestform >

/******************************************************************************************************
 ***********************************ClientRegistrationFactory****************************************
 ******************************************************************************************************
 */

class ClientRegistrationHandler;

class ClientRegistrationFactory {
	std::map<ConfigParams::ClientType, boost::shared_ptr<sim_mob::ClientRegistrationHandler> > ClientRegistrationHandlerMap;
public:
	ClientRegistrationFactory();
	boost::shared_ptr<sim_mob::ClientRegistrationHandler> getHandler(ConfigParams::ClientType type);
	virtual ~ClientRegistrationFactory();
};

/******************************************************************************************************
 ***********************************ClientRegistrationPublisher****************************************
 ******************************************************************************************************
 */
class ClientRegistrationPublisher : public sim_mob::event::EventPublisher
{
public:
	ClientRegistrationPublisher();
	virtual ~ClientRegistrationPublisher();
};

/******************************************************************************************************
 ***********************************ClientRegistrationHandler****************************************
 ******************************************************************************************************
 */
class Broker;

class ClientRegistrationHandler {
	ConfigParams::ClientType type;
	static ClientRegistrationPublisher registrationPublisher;
public:
	ClientRegistrationHandler(ConfigParams::ClientType);
	virtual bool handle(sim_mob::Broker&, sim_mob::ClientRegistrationRequest) = 0;
	static sim_mob::event::EventPublisher & getPublisher();
	virtual ~ClientRegistrationHandler();
};

/******************************************************************************************************
 ***********************************ClientRegistrationEventArgs****************************************
 ******************************************************************************************************
 */
class ClientHandler;
class ClientRegistrationEventArgs: public sim_mob::event::EventArgs {
	boost::shared_ptr<ClientHandler> client;
	ConfigParams::ClientType type;
public:
	ClientRegistrationEventArgs(ConfigParams::ClientType, boost::shared_ptr<ClientHandler>&);
	boost::shared_ptr<ClientHandler> getClient() const;
	ConfigParams::ClientType getClientType() const;
	virtual ~ClientRegistrationEventArgs();
};

}//namespace sim_mob

