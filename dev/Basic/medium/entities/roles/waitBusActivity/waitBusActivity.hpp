//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include "entities/roles/Role.hpp"
#include "waitBusActivityFacets.hpp"

namespace sim_mob
{

class Agent;
class Person;
class BusStop;

namespace medium
{

class WaitBusActivityBehavior;
class WaitBusActivityMovement;

enum Decision{MAKE_DECISION_NORESULT, MAKE_DECISION_BOARDING};
/**
 * A medium-term WaitBusActivity.
 * \author Seth N. Hetu
 * \author zhang huai peng
 */
class WaitBusActivity : public sim_mob::Role {
public:

	explicit WaitBusActivity(Agent* parent, MutexStrategy mtxStrat, sim_mob::medium::WaitBusActivityBehavior* behavior = nullptr, sim_mob::medium::WaitBusActivityMovement* movement = nullptr);

	virtual ~WaitBusActivity() {}

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	virtual void make_frame_tick_params(timeslice now) { throw std::runtime_error("make_frame_tick_params not implemented in WaitBusActivity."); }
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams() { throw std::runtime_error("getSubscriptionParams not implemented in WaitBusActivity."); }

	void increaseWaitingTime(unsigned int ms);
	void setStopSite(sim_mob::BusStop* stop);

	void setMakeDecisionResult(Decision decision);
	Decision getMakeDecisionResult();

private:
	friend class WaitBusActivityBehavior;
	friend class WaitBusActivityMovement;

	unsigned int waitingTimeAtBusStop;

	BusStop* stopSite;
	Decision decisionResult;
};


}}
