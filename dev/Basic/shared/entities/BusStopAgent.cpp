/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BusStopAgent.hpp"

using std::vector;

using namespace sim_mob;

typedef Entity::UpdateStatus UpdateStatus;

BusStopAgent::All_BusStopAgents BusStopAgent::all_BusstopAgents_;

void sim_mob::BusStopAgent::RegisterNewBusStopAgent(BusStop const & busstop, const MutexStrategy& mtxStrat)
{
	//BusController* busctrller = new sim_mob::BusController(-1, mtxStrat);
	BusStopAgent * sig_ag = new BusStopAgent(busstop, mtxStrat);
	all_BusstopAgents_.push_back(sig_ag);
}

void sim_mob::BusStopAgent::buildSubscriptionList(vector<BufferedBase*>& subsList)
{
	Agent::buildSubscriptionList(subsList);
}

bool sim_mob::BusStopAgent::frame_init(timeslice now)
{
	return true;
}

void sim_mob::BusStopAgent::frame_output(timeslice now)
{

}

Entity::UpdateStatus sim_mob::BusStopAgent::frame_tick(timeslice now)
{
	return Entity::UpdateStatus::Continue;
}
