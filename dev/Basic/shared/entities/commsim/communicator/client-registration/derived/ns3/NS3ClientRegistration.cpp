//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * NS3ClientRegistration.cpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#include "NS3ClientRegistration.hpp"
#include "entities/commsim/communicator/event/subscribers/base/ClientHandler.hpp"
namespace sim_mob {

NS3ClientRegistration::NS3ClientRegistration(/*ConfigParams::ClientType type_) : ClientRegistrationHandler(type_*/) {
	// TODO Auto-generated constructor stub

}

bool NS3ClientRegistration::handle(sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest request){
	return true;
}
NS3ClientRegistration::~NS3ClientRegistration() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
