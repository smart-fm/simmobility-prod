/*
 * TrainStationAgent.hpp
 *
 *  Created on: Feb 24, 2016
 *      Author: zhang huai peng
 */

#ifndef TRAINSTATIONAGENT_HPP_
#define TRAINSTATIONAGENT_HPP_
#include "entities/Agent.hpp"
#include "geospatial/network/PT_Stop.hpp"
namespace sim_mob {
namespace medium
{
class TrainStationAgent : public sim_mob::Agent {
public:
	TrainStationAgent();
	virtual ~TrainStationAgent();
};
}
} /* namespace sim_mob */

#endif /* TRAINSTATIONAGENT_HPP_ */
