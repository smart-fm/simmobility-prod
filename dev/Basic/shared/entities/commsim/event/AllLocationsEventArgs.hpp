//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * AllLocationsEventArgs.hpp
 *
 *  Created on: Jul 5, 2013
 *      Author: vahid
 */

#pragma once

#include<iostream>

#include "event/EventListener.hpp"
#include "entities/commsim/serialization/JsonParser.hpp"
#include "entities/commsim/broker/Broker.hpp"
#include "entities/commsim/event/JsonSerializableEventArgs.hpp"

namespace sim_mob {

DECLARE_CUSTOM_CALLBACK_TYPE(AllLocationsEventArgs)
class AllLocationsEventArgs: public sim_mob::comm::JsonSerializableEventArgs {
	sim_mob::AgentsList  &registered_Agents;

	void TOJSON(sim_mob::Agent* agent,Json::Value &loc)const;
public:
	AllLocationsEventArgs(sim_mob::AgentsList &registered_Agents_);
	virtual Json::Value toJSON()const;
	virtual ~AllLocationsEventArgs();
};

} /* namespace sim_mob */
