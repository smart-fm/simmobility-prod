//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "event/EventListener.hpp"
#include "entities/commsim/serialization/JsonParser.hpp"
#include "entities/commsim/event/JsonSerializableEventArgs.hpp"

namespace sim_mob {
class Agent;
DECLARE_CUSTOM_CALLBACK_TYPE(RegionsAndPathEventArgs)
class RegionsAndPathEventArgs : public sim_mob::comm::JsonSerializableEventArgs {
public:
	RegionsAndPathEventArgs(const sim_mob::Agent *);
	virtual ~RegionsAndPathEventArgs();

	virtual Json::Value toJSON()const;

private:
	const sim_mob::Agent *agent;

};

} /* namespace sim_mob */
