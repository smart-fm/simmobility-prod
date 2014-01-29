//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "NewClientHandler.hpp"

#include "entities/commsim/connection/WhoAreYouProtocol.hpp"
#include "entities/commsim/connection/ConnectionServer.hpp"
#include "entities/commsim/Broker.hpp"

using namespace sim_mob;


//handler implementation
void sim_mob::roadrunner::NewClientHandler::handle(sim_mob::comm::MsgPtr message_, sim_mob::Broker* broker, boost::shared_ptr<sim_mob::ConnectionHandler> caller)
{
	//
	//TODO: It seems that we can proceed as follows:
	//      1) Send a message to the client, like in ConnectionServer::handleNew(). Basically, this sends WHOAREYOU.
	//      2) When WHOAMI is returned, the WhoAreYou_response_handler() will call ConnectionServer::RequestClientRegistration(),
	//         which will add the client to "clientRegistrationWaitingList".
	//      3) Take care of Broker::processClientRegistrationRequests(). Basically, the "RequestClientRegistration()" method should
	//         *already* have a valid session (and socket, etc.), but we'll need to make sure it carries through here.
	//      4) Debug: some of these data structures (Session, etc.) might not be thread-safe.
	//      5) Make sure we can still connect to the original IP address directly if required.
	//      6) More debugging: make sure messages are filtered properly to the correct Agents.
	//

	//New session, copy the old.
	session_ptr newSess(new sim_mob::Session(broker->getConnectionServer()->io_service_));

	//TODO: Actually, this might be sufficient, since the session is a shared pointer.
	WhoAreYouProtocol *registration = new sim_mob::WhoAreYouProtocol(newSess, *broker->getConnectionServer());
	registration->start();

	//throw std::runtime_error("NEW_CLIENT not yet supported.");
}
