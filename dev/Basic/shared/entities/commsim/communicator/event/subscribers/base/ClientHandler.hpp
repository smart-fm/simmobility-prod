/*
 * ClientHandler.hpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#ifndef CLIENTHANDLER_HPP_
#define CLIENTHANDLER_HPP_
#include <boost/shared_ptr.hpp>
#include <map>
#include<set>
#include "event/EventListener.hpp"
#include "entities/commsim/communicator/event/TimeEventArgs.hpp"
#include "entities/commsim/communicator/service/services.hpp"


namespace sim_mob {
//Forward Declarations
class ClientHandler;
class Broker;
class ConnectionHandler;
class JCommunicationSupport;
class Agent;

class ClientHandler: public sim_mob::EventListener {
	sim_mob::Broker & broker;
public:
	ClientHandler(sim_mob::Broker &);
	boost::shared_ptr<sim_mob::ConnectionHandler > cnnHandler;
	sim_mob::JCommunicationSupport* JCommunicationSupport_; //represents a Role, so dont use a boost::share_ptr whose object is created somewhere else. it is dangerous
	const sim_mob::Agent* agent;//same: dont use a boost::share_ptr whose object is created somewhere else. it is dangerous
	unsigned int clientID;
	unsigned int client_type; //ns3, android emulator, FMOD etc
	std::set<sim_mob::SIM_MOB_SERVICE> requiredServices;
	sim_mob::Broker &getBroker();
	virtual ~ClientHandler();
};

} /* namespace sim_mob */
#endif /* CLIENTHANDLER_HPP_ */
