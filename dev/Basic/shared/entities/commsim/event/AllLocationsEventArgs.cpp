/*
 * AllLocationsEventArgs.cpp
 *
 *  Created on: Jul 5, 2013
 *      Author: vahid
 */

#include "AllLocationsEventArgs.hpp"
//#include "entities/commsim/broker/Broker.hpp"
namespace sim_mob {
AllLocationsEventArgs::AllLocationsEventArgs(sim_mob::AgentsMap<std::string>::type  &registeredAgents_):registeredAgents(registeredAgents_) {
	// TODO Auto-generated constructor stub
}

Json::Value AllLocationsEventArgs::ToJSON()const{
	Json::Value loc = JsonParser::createMessageHeader(msg_header("0", "SIMMOBILITY", "ALL_LOCATIONS_DATA"));
	sim_mob::AgentsMap<std::string>::pair pair;
	BOOST_FOREACH(pair, registeredAgents)
	{
		loc["LOCATIONS"].append(JsonParser::makeLocationArrayElement(pair.first->getId(),pair.first->xPos.get(), pair.first->yPos.get()));
	}
//	Print() << "AllLocationsEventArgs::ToJSON's cook:\n '" << loc.toStyledString() << "'" << std::endl;
	return loc;
}

AllLocationsEventArgs::~AllLocationsEventArgs() {
	// TODO Auto-generated destructor stub
}
}

