/*
 * ClientRegistration.cpp
 *
 *  Created on: Jul 11, 2013
 *      Author: vahid
 */

#include "ClientRegistration.hpp"
namespace sim_mob
{
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

}//namespace sim_mob
