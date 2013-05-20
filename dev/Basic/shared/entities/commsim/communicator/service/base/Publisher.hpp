/*
 * Publisher.hpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

#ifndef PUBLISHER_HPP_
#define PUBLISHER_HPP_
#include "entities/commsim/communicator/broker/Broker.hpp"
#include "metrics/Frame.hpp"
namespace sim_mob {
class Publisher {
private:
	CAPABILITY myCapability;
public:
	Publisher();
	virtual void publish(sim_mob::Broker *, sim_mob::subscription&, timeslice) = 0;
	virtual ~Publisher();
};

} /* namespace sim_mob */
#endif /* PUBLISHER_HPP_ */
