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


/**
 * ClientRegistrationRequest class. No documentation provided.
 */
class ClientRegistrationRequest {
public:
	ClientRegistrationRequest(const ClientRegistrationRequest& other);
	ClientRegistrationRequest();

	std::string clientID;
	std::string client_type; //ns3, android emulator, FMOD etc
	std::set<sim_mob::Services::SIM_MOB_SERVICE> requiredServices;
	session_ptr session_;

	//ClientRegistrationRequest & operator=(const ClientRegistrationRequest & rhs);
};


class ClientRegistrationHandler;


/**
 * ClientRegistrationFactory class. No documentation provided.
 */
/*class ClientRegistrationFactory {
	std::map<comm::ClientType, boost::shared_ptr<sim_mob::ClientRegistrationHandler> > ClientRegistrationHandlerMap;
public:
	ClientRegistrationFactory();
	virtual ~ClientRegistrationFactory();

	///gets a handler either from a cache or by creating a new one
	boost::shared_ptr<sim_mob::ClientRegistrationHandler> getHandler(comm::ClientType type);
};*/


/**
 * ClientRegistrationPublisher class. No documentation provided.
 */
class ClientRegistrationPublisher : public sim_mob::event::EventPublisher
{
public:
	virtual ~ClientRegistrationPublisher() {}
};


class Broker;

/**
 * ClientRegistrationHandler class. No documentation provided.
 */
class ClientRegistrationHandler {
	comm::ClientType type;
	static ClientRegistrationPublisher registrationPublisher;
public:
	ClientRegistrationHandler(comm::ClientType);
	virtual bool handle(sim_mob::Broker&, sim_mob::ClientRegistrationRequest) = 0;
	static sim_mob::event::EventPublisher & getPublisher();
	virtual ~ClientRegistrationHandler();
};



class ClientHandler;

/**
 * ClientRegistrationEventArgs class. No documentation provided.
 */
class ClientRegistrationEventArgs: public sim_mob::event::EventArgs {
	boost::shared_ptr<ClientHandler> client;
	comm::ClientType type;
public:
	ClientRegistrationEventArgs(comm::ClientType, boost::shared_ptr<ClientHandler>&);
	boost::shared_ptr<ClientHandler> getClient() const;
	comm::ClientType getClientType() const;
	virtual ~ClientRegistrationEventArgs();
};

}//namespace sim_mob

