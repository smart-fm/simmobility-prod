/*
 * ClientRegistration.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#pragma once

#include <set>
#include<map>
#include "entities/commsim/communicator/service/services.hpp"
#include <boost/shared_ptr.hpp>
namespace sim_mob
{
//Forward declaration
class Session;
struct ClientRegistrationRequest
{
	std::string clientID;
	std::string client_type; //ns3, android emulator, FMOD etc
	std::set<sim_mob::SIM_MOB_SERVICE> requiredServices;
	boost::shared_ptr<Session> session_;
	ClientRegistrationRequest(const ClientRegistrationRequest& other)
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
	ClientRegistrationRequest()
	{
		requiredServices.clear();
	}
	ClientRegistrationRequest & operator=(const ClientRegistrationRequest & rhs)
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

};

typedef std::multimap<std::string,ClientRegistrationRequest > ClientWaitList; //<client type,registrationrequestform >
}
