//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * \file Vehicle.hpp
 *
 * \author Zhang huai Peng
 */

#pragma once

#include "entities/Agent.hpp"
#include "event/SystemEvents.hpp"
#include "event/args/EventArgs.hpp"
#include "message/MessageBus.hpp"
#include "event/EventPublisher.hpp"

namespace sim_mob {
class BusStop;

namespace medium {
class BusDriver;

enum {
	MSG_DECISION_WAITINGPERSON_BOARDING = 6000000,
	MSG_DECISION_PASSENGER_ALIGHTING = 6000001
};

/**
 * Subclass wraps a bus driver into message so as to make boarding decision.
 * This is to allow it to function as an message callback parameter.
 */
class WaitingPeopleBoardingDecisionMessageArgs : public messaging::Message {
public:
	WaitingPeopleBoardingDecisionMessageArgs(BusDriver* driver):busDriver(driver){;}
	virtual ~WaitingPeopleBoardingDecisionMessageArgs() {}
	BusDriver* busDriver;
};

/**
 * Subclass wraps a bus stop into message so as to make alighting decision.
 * This is to allow it to function as an message callback parameter.
 */
class PassengerAlightingDecisionMessageArgs : public messaging::Message {
public:
	PassengerAlightingDecisionMessageArgs(const BusStop* stop):nextStop(stop){;}
	virtual ~PassengerAlightingDecisionMessageArgs() {}
	const BusStop* nextStop;
};
}
}
