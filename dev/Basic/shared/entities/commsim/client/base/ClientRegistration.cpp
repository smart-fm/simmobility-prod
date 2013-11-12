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
//#include "entities/commsim/client/base/ClinetRegistrationHandler.hpp"

#include "entities/commsim/client/derived/android/AndroidClientRegistration.hpp"
#include "entities/commsim/client/derived/ns3/NS3ClientRegistration.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"

#include <boost/assign/list_of.hpp>

namespace sim_mob {

/******************************************************************************************************
 ***********************************ClientRegistrationFactory****************************************
 ******************************************************************************************************
 */
ClientRegistrationFactory::ClientRegistrationFactory() {
	//TODO: This might be superfluous; the static initializer already handles it.
	sim_mob::Services::ClientTypeMap = boost::assign::map_list_of("ANDROID_EMULATOR", ConfigParams::ANDROID_EMULATOR)("ConfigParams::NS3_SIMULATOR", ConfigParams::NS3_SIMULATOR);

}

//gets a handler either from a cache or by creating a new one
boost::shared_ptr<sim_mob::ClientRegistrationHandler> ClientRegistrationFactory::getHandler(ConfigParams::ClientType type)
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

ClientRegistrationFactory::~ClientRegistrationFactory() {
	// TODO Auto-generated destructor stub
}

/******************************************************************************************************
 ***********************************ClientRegistrationRequest****************************************
 ******************************************************************************************************
 */

ClientRegistrationRequest::ClientRegistrationRequest(const ClientRegistrationRequest& other)
	:
		clientID(other.clientID)
		,client_type(other.client_type)
	{
		if(other.requiredServices.size())
		{
			requiredServices = other.requiredServices;
		}
		if(other.session_)
		{
			session_ = other.session_;
		}
	}
ClientRegistrationRequest::ClientRegistrationRequest()
	{
		requiredServices.clear();
	}
	ClientRegistrationRequest & ClientRegistrationRequest::operator=(const ClientRegistrationRequest & rhs)
	{

		clientID = rhs.clientID;
		client_type = rhs.client_type;
		if(rhs.requiredServices.size())
		{
			requiredServices = rhs.requiredServices;
		}
		if(session_)
		{
			session_ = rhs.session_;
		}
		return *this;
	}

	/******************************************************************************************************
	 ***********************************ClientRegistrationPublisher****************************************
	 ******************************************************************************************************
	 */

	ClientRegistrationPublisher::ClientRegistrationPublisher(/*ConfigParams::ClientType type, ClientRegistrationHandler* client*/)
//			:type(type), client(client)
	{

	}

	ClientRegistrationPublisher::~ClientRegistrationPublisher()
	{

	}
	/******************************************************************************************************
	 ***********************************ClientRegistrationHandler****************************************
	 ******************************************************************************************************
	 */

		ClientRegistrationPublisher ClientRegistrationHandler::registrationPublisher;

		ClientRegistrationHandler::ClientRegistrationHandler(ConfigParams::ClientType type):type(type) {
			registrationPublisher.RegisterEvent(type);
		}
		sim_mob::event::EventPublisher & ClientRegistrationHandler::getPublisher() {
			return registrationPublisher;
		}
		ClientRegistrationHandler::~ClientRegistrationHandler() {
			// TODO Auto-generated destructor stub
		}

	/******************************************************************************************************
	 ***********************************ClientRegistrationEventArgs****************************************
	 ******************************************************************************************************
	 */

		ClientRegistrationEventArgs::ClientRegistrationEventArgs(ConfigParams::ClientType type, boost::shared_ptr<ClientHandler> &client):type(type), client(client) {}
		boost::shared_ptr<ClientHandler> ClientRegistrationEventArgs::getClient() const
		{
			return client;
		}
		ConfigParams::ClientType ClientRegistrationEventArgs::getClientType() const
		{
			return type;
		}
		ClientRegistrationEventArgs::~ClientRegistrationEventArgs(){}

} /* namespace sim_mob */


