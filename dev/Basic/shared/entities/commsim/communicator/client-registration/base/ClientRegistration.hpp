/*
 * ClientRegistration.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#ifndef CLIENTREGISTRATION_HPP_
#define CLIENTREGISTRATION_HPP_


struct ClientRegistrationRequest
{
	unsigned int clientID;
	unsigned int client_type; //ns3, android emulator, FMOD etc
	std::set<sim_mob::SIM_MOB_SERVICE> requiredServices;
	sim_mob::session_ptr session_;
};
//Forward Declaration
template<class T>
class JCommunicationSupport;

struct registeredClient {
	boost::shared_ptr<sim_mob::ConnectionHandler > cnnHandler;
	sim_mob::JCommunicationSupport* JCommunicationSupport_; //represents a Role, so dont use a boost::share_ptr whose object is created somewhere else. it is dangerous
	const sim_mob::Agent* agent;
	unsigned int clientID;
	unsigned int client_type; //ns3, android emulator, FMOD etc
	std::set<SIM_MOB_SERVICE> requiredServices;
};

typedef std::multimap<unsigned int,ClientRegistrationRequest > ClientWaitList; //<client type,registrationrequestform >
#endif /* CLIENTREGISTRATION_HPP_ */
