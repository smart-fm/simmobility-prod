//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "event/EventListener.hpp"
#include "entities/commsim/comm_support/AgentCommUtility.hpp"
#include "entities/commsim/event/BaseCommsimEventArgs.hpp"
#include "entities/commsim/event/TimeEventArgs.hpp"
#include "entities/commsim/event/LocationEventArgs.hpp"
#include "entities/commsim/event/AllLocationsEventArgs.hpp"
#include "entities/commsim/service/Services.hpp"

namespace sim_mob {
class Broker;
class ConnectionHandler;
class JsonSerializableEventArgs;

template<class T>
class AgentCommUtility;
class Agent;

class ClientHandler;
class ClientHandler: public sim_mob::event::EventListener, public boost::enable_shared_from_this<ClientHandler> {
public:
	///conn is the connection handler this client can use to send messages.
	ClientHandler(sim_mob::Broker& broker, boost::shared_ptr<sim_mob::ConnectionHandler> conn, const sim_mob::Agent* agent, std::string clientId);
	virtual ~ClientHandler();

	//event functions:
	void sendSerializedMessageToBroker(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const sim_mob::BaseCommsimEventArgs& args);

	//Getters/setters
	void setRequiredServices(const std::set<sim_mob::Services::SIM_MOB_SERVICE>& requiredServices);
	const std::set<sim_mob::Services::SIM_MOB_SERVICE>& getRequiredServices();
	bool isValid() const;
	void setValidation(bool);
	sim_mob::Broker &getBroker();



private:
	std::set<sim_mob::Services::SIM_MOB_SERVICE> requiredServices;
	sim_mob::Broker& broker;
	bool valid;

public: //TODO: Some of these should clearly be private; however, for now they are all accessed in too many places.
	boost::shared_ptr<sim_mob::ConnectionHandler> connHandle;
	const sim_mob::Agent* agent;//same: dont use a boost::share_ptr whose object is created somewhere else. it is dangerous
	std::string clientId;
};

} /* namespace sim_mob */
