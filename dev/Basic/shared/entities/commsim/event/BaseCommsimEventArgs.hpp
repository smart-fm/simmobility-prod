//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>

#include "event/args/EventArgs.hpp"

namespace sim_mob {

/**
 * Inherit from this class to indicate that your subclass can serialize itself for commsim transmission.
 * This functionality is used by the Event manager, so this class also subclasses EventArgs.
 * Essentially, anything that uses a subclass will call "serialize()" during "onEvent()".
 */
class BaseCommsimEventArgs : public sim_mob::event::EventArgs {
public:
	virtual ~BaseCommsimEventArgs() {}
	virtual std::string serialize() const = 0;
};

}
