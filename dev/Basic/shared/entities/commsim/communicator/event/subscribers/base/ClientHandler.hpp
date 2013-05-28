/*
 * ClientHandler.hpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#ifndef CLIENTHANDLER_HPP_
#define CLIENTHANDLER_HPP_

#include "event/EventListener.hpp"
#include "entities/commsim/communicator/event/TimeEventArgs.hpp"
#include <boost/shared_ptr.hpp>
#include "entities/commsim/communicator/connection/Connection.hpp"

namespace sim_mob {

class ClientHandler: public sim_mob::EventListener {
public:

	boost::shared_ptr<sim_mob::ConnectionHandler > cnnHandler;
	sim_mob::JCommunicationSupport* JCommunicationSupport_; //represents a Role, so dont use a boost::share_ptr whose object is created somewhere else. it is dangerous
	const sim_mob::Agent* agent;
	unsigned int clientID;
	unsigned int client_type; //ns3, android emulator, FMOD etc

	ClientHandler();
	virtual ~ClientHandler();
};

} /* namespace sim_mob */
#endif /* CLIENTHANDLER_HPP_ */
