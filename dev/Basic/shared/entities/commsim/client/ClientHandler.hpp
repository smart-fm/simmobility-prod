//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

//#include "event/EventListener.hpp"
#include "entities/commsim/service/Services.hpp"

namespace sim_mob {
class BrokerBase;
class ConnectionHandler;
class JsonSerializableEventArgs;
class Agent;

class ClientHandler;
class ClientHandler: /*public sim_mob::event::EventListener,*/ public boost::enable_shared_from_this<ClientHandler> {
public:
	///conn is the connection handler this client can use to send messages.
	ClientHandler(BrokerBase& broker, boost::shared_ptr<sim_mob::ConnectionHandler> conn, const sim_mob::Agent* agent, std::string clientId);
	virtual ~ClientHandler();

	//Getters/setters
	void setRequiredServices(const std::set<sim_mob::Services::SIM_MOB_SERVICE>& requiredServices);
	const std::set<sim_mob::Services::SIM_MOB_SERVICE>& getRequiredServices();
	bool isValid() const;
	void setValidation(bool);


private:
	std::set<sim_mob::Services::SIM_MOB_SERVICE> requiredServices;
	sim_mob::BrokerBase& broker;
	bool valid;

public: //TODO: Some of these should clearly be private; however, for now they are all accessed in too many places.
	boost::shared_ptr<sim_mob::ConnectionHandler> connHandle;
	const sim_mob::Agent* agent;
	std::string clientId;

public:
	//TODO: These are kind of hackish, but it makes sense (Clients register for these events).
	//      There may be a better way to do this later.
	bool regisTime; ///<Has this Client registered for TIME_INFO events?
	bool regisLocation; ///<Has this Client registered for LOCATION_UPDATE events?
	bool regisRegionPath; ///<Has this Client registered for REGIONS_AND_PATHS events? (typically only RoadRunner)
	bool regisAllLocations; ///<Has this Client registered for ALL_LOCATIONS events? (typically only ns-3)
};

}
