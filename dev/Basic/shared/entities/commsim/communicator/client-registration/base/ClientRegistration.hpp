/*
 * ClientRegistration.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#ifndef CLIENTREGISTRATION_HPP_
#define CLIENTREGISTRATION_HPP_
#include <set>
#include "entities/commsim/communicator/service/services.hpp"
#include <boost/shared_ptr.hpp>
namespace sim_mob
{
//Forward declaration
class Session;
struct ClientRegistrationRequest
{
	unsigned int clientID;
	unsigned int client_type; //ns3, android emulator, FMOD etc
	std::set<sim_mob::SIM_MOB_SERVICE> requiredServices;
	boost::shared_ptr<Session> session_;
};

typedef std::multimap<unsigned int,ClientRegistrationRequest > ClientWaitList; //<client type,registrationrequestform >
}
#endif /* CLIENTREGISTRATION_HPP_ */
