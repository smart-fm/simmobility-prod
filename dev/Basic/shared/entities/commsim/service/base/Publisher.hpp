/*
 * Publisher.hpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

#ifndef PUBLISHER_HPP_
#define PUBLISHER_HPP_
#include "entities/commsim/service/services.hpp"
#include <boost/assign/list_of.hpp>
#include "entities/commsim/broker/Broker.hpp"
#include "metrics/Frame.hpp"
#include "event/EventPublisher.hpp"
namespace sim_mob {
class Publisher : public sim_mob::event::EventPublisher {
private:
//	sim_mob::SIM_MOB_SERVICE myService;
public:
	Publisher();
//	virtual void publish(sim_mob::Broker&, sim_mob::registeredClient&, timeslice) = 0;
	virtual ~Publisher();
};

} /* namespace sim_mob */
#endif /* PUBLISHER_HPP_ */
