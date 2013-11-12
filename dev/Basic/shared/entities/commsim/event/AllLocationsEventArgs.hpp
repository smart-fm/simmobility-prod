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

#include "event/args/EventArgs.hpp"
#include "event/EventListener.hpp"
#include<iostream>
#include "entities/commsim/serialization/Serialization.hpp"
#include "entities/commsim/broker/Broker.hpp"

namespace sim_mob {

DECLARE_CUSTOM_CALLBACK_TYPE(AllLocationsEventArgs)
class AllLocationsEventArgs: public sim_mob::event::EventArgs {
	sim_mob::AgentsList  &registered_Agents;

	void TOJSON(sim_mob::Agent* agent,Json::Value &loc)const;
public:
	AllLocationsEventArgs(sim_mob::AgentsList &registered_Agents_);
	Json::Value ToJSON()const;
	virtual ~AllLocationsEventArgs();
};

} /* namespace sim_mob */
