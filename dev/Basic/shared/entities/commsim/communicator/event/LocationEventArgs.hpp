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
#include<iostream>
#include "entities/commsim/communicator/serialization/Serialization.hpp"

namespace sim_mob {
class Agent;
DECLARE_CUSTOM_CALLBACK_TYPE(LocationEventArgs)
class LocationEventArgs: public sim_mob::event::EventArgs {
public:
	const sim_mob::Agent *agent;
	LocationEventArgs(const sim_mob::Agent *);
	//todo should be a virtual from a base class
	/*std::string*/ Json::Value ToJSON()const;
	virtual ~LocationEventArgs();
};

} /* namespace sim_mob */
#endif /* LOCATIONEVENTARGS_HPP_ */
