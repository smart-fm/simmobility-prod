//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/commsim/event/BaseCommsimEventArgs.hpp"
#include "metrics/Frame.hpp"

namespace sim_mob {

/**
 * Contains event arguments for a timeslice update.
 */
class TimeEventArgs: public BaseCommsimEventArgs {
public:
	TimeEventArgs(timeslice time);
	virtual ~TimeEventArgs();

	virtual std::string serialize() const;
private:
	timeslice time;
};


}
