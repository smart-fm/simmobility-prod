/*
 * LocationEventArgs.hpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#ifndef LOCATIONEVENTARGS_HPP_
#define LOCATIONEVENTARGS_HPP_

#include "event/args/EventArgs.hpp"
#include "event/EventListener.hpp"

namespace sim_mob {
class Agent;
DECLARE_CUSTOM_CALLBACK_TYPE(LocationEventArgs)
class LocationEventArgs: public sim_mob::EventArgs {
	sim_mob::Agent *agent;
public:
	LocationEventArgs(sim_mob::Agent *);
	virtual ~LocationEventArgs();
};

} /* namespace sim_mob */
#endif /* LOCATIONEVENTARGS_HPP_ */
