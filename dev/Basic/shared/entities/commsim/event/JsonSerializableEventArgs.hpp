//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "event/args/EventArgs.hpp"
#include "entities/commsim/event/JsonSerializable.hpp"

namespace sim_mob {
namespace comm {

/**
 * Subclasses both EventArgs and JsonSerializable. This is to allow it to function as an Event callback parameter.
 */
class JsonSerializableEventArgs : public sim_mob::event::EventArgs, public sim_mob::comm::JsonSerializable {
public:
	virtual ~JsonSerializableEventArgs() {}
};

}}
