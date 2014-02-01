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

#include "entities/commsim/client/derived/android/AndroidClientRegistration.hpp"
#include "entities/commsim/client/derived/ns3/NS3ClientRegistration.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"

using namespace sim_mob;


/******************************************************************************************************
 ***********************************ClientRegistrationHandler****************************************
 ******************************************************************************************************
 */

ClientRegistrationPublisher sim_mob::ClientRegistrationHandler::registrationPublisher;

sim_mob::ClientRegistrationHandler::ClientRegistrationHandler(/*comm::ClientType type):type(type*/)
{
	//registrationPublisher.registerEvent(type);
}

void sim_mob::ClientRegistrationHandler::postProcess(sim_mob::Broker& broker){

}

sim_mob::ClientRegistrationHandler::~ClientRegistrationHandler()
{
}

/******************************************************************************************************
 ***********************************ClientRegistrationEventArgs****************************************
 ******************************************************************************************************
 */

sim_mob::ClientRegistrationEventArgs::ClientRegistrationEventArgs(comm::ClientType type, boost::shared_ptr<ClientHandler> &client) :
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

comm::ClientType sim_mob::ClientRegistrationEventArgs::getClientType() const
{
	return type;
}


