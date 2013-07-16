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

#include <boost/assign/list_of.hpp>

namespace sim_mob {

ClientRegistrationFactory::ClientRegistrationFactory() {
	ClientTypeMap = boost::assign::map_list_of("ANDROID_EMULATOR", ConfigParams::ANDROID_EMULATOR)("ConfigParams::NS3_SIMULATOR", ConfigParams::NS3_SIMULATOR);
	// TODO Auto-generated constructor stub

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

	ClientRegistrationHandler::ClientRegistrationHandler() {
		// TODO Auto-generated constructor stub

	}

	ClientRegistrationHandler::~ClientRegistrationHandler() {
		// TODO Auto-generated destructor stub
	}


} /* namespace sim_mob */


