/*
 * BusStopAgent.cpp
 *
 *  Created on: 17 Apr, 2014
 *      Author: zhang
 */

#include <entities/busStopAgent/BusStopAgent.hpp>

namespace sim_mob
{

namespace medium
{


BusStopAgent::BusStopAgent(const MutexStrategy& mtxStrat, int id):Agent(mtxStrat, id){
	// TODO Auto-generated constructor stub

}

BusStopAgent::~BusStopAgent() {
	// TODO Auto-generated destructor stub
}

void BusStopAgent::onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args){
	Agent::onEvent(eventId, ctxId, sender, args);
}


}

}

