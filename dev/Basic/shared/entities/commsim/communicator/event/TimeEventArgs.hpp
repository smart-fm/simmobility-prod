/*
 * TimeEventArgs.hpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#ifndef TIMEEVENTARGS_HPP_
#define TIMEEVENTARGS_HPP_

#include "event/args/EventArgs.hpp"
#include "event/EventListener.hpp"
#include "metrics/Frame.hpp"
#include <iostream>

namespace sim_mob {
DECLARE_CUSTOM_CALLBACK_TYPE(TimeEventArgs)
class TimeEventArgs: public sim_mob::EventArgs {
public:
	TimeEventArgs(timeslice time);
	virtual ~TimeEventArgs();
	std::string ToJSON()const;
	timeslice time;
};


} /* namespace sim_mob */
#endif /* TIMEEVENTARGS_HPP_ */
