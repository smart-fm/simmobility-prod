/*
 * LocationPublisher.hpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

#ifndef LOCATIONPUBLISHER_HPP_
#define LOCATIONPUBLISHER_HPP_

#include "entities/commsim/communicator/service/base/Publisher.hpp"

namespace sim_mob {

class LocationPublisher: public sim_mob::Publisher {
public:
	LocationPublisher();
//	void publish(sim_mob::Broker &,sim_mob::registeredClient &, timeslice);
	virtual ~LocationPublisher();
};

} /* namespace sim_mob */
#endif /* LOCATIONPUBLISHER_HPP_ */
