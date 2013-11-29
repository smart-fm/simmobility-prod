//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ClientHandler.hpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#pragma once

#include <map>
#include <set>

#include <boost/shared_ptr.hpp>

#include "event/EventListener.hpp"
#include "entities/commsim/event/TimeEventArgs.hpp"
#include "entities/commsim/event/LocationEventArgs.hpp"
#include "entities/commsim/event/AllLocationsEventArgs.hpp"
#include "entities/commsim/service/Services.hpp"


namespace sim_mob {
//Forward Declarations
class ClientHandler;
class Broker;
class ConnectionHandler;

template<class T>
class AgentCommUtility;
class Agent;

class ClientHandler: public sim_mob::event::EventListener {
	sim_mob::Broker & broker;
	bool valid;
public:
	ClientHandler(sim_mob::Broker &);
	boost::shared_ptr<sim_mob::ConnectionHandler > cnnHandler;
	sim_mob::AgentCommUtilityBase* AgentCommUtility_; //represents a Role, so dont use a boost::share_ptr whose object is created somewhere else. it is dangerous
	const sim_mob::Agent* agent;//same: dont use a boost::share_ptr whose object is created somewhere else. it is dangerous
	std::string clientID;
	unsigned int client_type; //ns3, android emulator, FMOD etc
	std::set<sim_mob::Services::SIM_MOB_SERVICE> requiredServices;
	sim_mob::Broker &getBroker();
	virtual ~ClientHandler();
	//event functions:
	void OnLocation(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const LocationEventArgs& args);
	void OnAllLocations(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const AllLocationsEventArgs& argums);
	void OnTime(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const TimeEventArgs& args);
	 bool isValid();
	 void setValidation(bool);
};

} /* namespace sim_mob */
