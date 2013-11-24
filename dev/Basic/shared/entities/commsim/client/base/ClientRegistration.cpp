//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ClientRegistration.cpp
 *
 *  Created on: Jul 15, 2013
 *      Author: vahid
 */


#include "ClientRegistration.hpp"

#include <boost/assign/list_of.hpp>

#include "entities/commsim/client/derived/android/AndroidClientRegistration.hpp"
#include "entities/commsim/client/derived/ns3/NS3ClientRegistration.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"

using namespace sim_mob;

/******************************************************************************************************
 ***********************************ClientRegistrationFactory****************************************
 ******************************************************************************************************
 */
sim_mob::ClientRegistrationFactory::ClientRegistrationFactory()
{
}

sim_mob::ClientRegistrationFactory::~ClientRegistrationFactory()
{
}

boost::shared_ptr<sim_mob::ClientRegistrationHandler> sim_mob::ClientRegistrationFactory::getHandler(ConfigParams::ClientType type)
{
	boost::shared_ptr<sim_mob::ClientRegistrationHandler> handler;
	//if handler is already registered && the registered handler is not null
	std::map<ConfigParams::ClientType, boost::shared_ptr<sim_mob::ClientRegistrationHandler> >::iterator it = ClientRegistrationHandlerMap.find(type);
	if(it != ClientRegistrationHandlerMap.end())
	{
		//get the handler ...
		handler = (*it).second;
	}
	else
	{
		//else, create a cache entry ...
		bool typeFound = true;
		switch(type)
		{
		case ConfigParams::ANDROID_EMULATOR:
			handler.reset(new sim_mob::AndroidClientRegistration());
			break;
		case ConfigParams::NS3_SIMULATOR:
			handler.reset(new sim_mob::NS3ClientRegistration);
			break;
		default:
			typeFound = false;
		}
		//register this baby
		if(typeFound)
		{
			ClientRegistrationHandlerMap[type] = handler;
		}
	}

	return handler;
}


/******************************************************************************************************
 ***********************************ClientRegistrationRequest****************************************
 ******************************************************************************************************
 */

sim_mob::ClientRegistrationRequest::ClientRegistrationRequest(const ClientRegistrationRequest& other) :
	clientID(other.clientID) ,client_type(other.client_type)
{
	//if(other.requiredServices.size() > 0)
	//{
		requiredServices = other.requiredServices;
	//}

	//NOTE: Copy semantics on shared_ptrs shouldn't require this check.
	//if(other.session_)
	//{
		session_ = other.session_;
	//}
}

sim_mob::ClientRegistrationRequest::ClientRegistrationRequest()
{
}

//The default operator= should already accomplish this. Please review. ~Seth
/*ClientRegistrationRequest& sim_mob::ClientRegistrationRequest::operator=(const ClientRegistrationRequest & rhs)
{
	clientID = rhs.clientID;
	client_type = rhs.client_type;
	if(rhs.requiredServices.size() > 0)
	{
		requiredServices = rhs.requiredServices;
	}
	if(session_)
	{
		session_ = rhs.session_;
	}
	return *this;
}*


/******************************************************************************************************
 ***********************************ClientRegistrationHandler****************************************
 ******************************************************************************************************
 */

ClientRegistrationPublisher sim_mob::ClientRegistrationHandler::registrationPublisher;

sim_mob::ClientRegistrationHandler::ClientRegistrationHandler(ConfigParams::ClientType type):type(type)
{
	registrationPublisher.RegisterEvent(type);
}

sim_mob::ClientRegistrationHandler::~ClientRegistrationHandler()
{
}

sim_mob::event::EventPublisher & sim_mob::ClientRegistrationHandler::getPublisher()
{
	return registrationPublisher;
}

/******************************************************************************************************
 ***********************************ClientRegistrationEventArgs****************************************
 ******************************************************************************************************
 */

sim_mob::ClientRegistrationEventArgs::ClientRegistrationEventArgs(ConfigParams::ClientType type, boost::shared_ptr<ClientHandler> &client) :
	type(type), client(client)
{
}

sim_mob::ClientRegistrationEventArgs::~ClientRegistrationEventArgs()
{
}

boost::shared_ptr<ClientHandler> ClientRegistrationEventArgs::getClient() const
{
	return client;
}

ConfigParams::ClientType sim_mob::ClientRegistrationEventArgs::getClientType() const
{
	return type;
}


