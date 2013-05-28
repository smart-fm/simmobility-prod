/*
 * TimePublisher.hpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

#ifndef TIMEPUBLISHER_HPP_
#define TIMEPUBLISHER_HPP_
#include "entities/commsim/communicator/service/base/Publisher.hpp"
#include "event/EventPublisher.hpp"
namespace sim_mob {

class TimePublisher: public sim_mob::Publisher  {
public:
	TimePublisher();
	virtual ~TimePublisher();
};

} /* namespace sim_mob */
#endif /* TIMEPUBLISHER_HPP_ */
