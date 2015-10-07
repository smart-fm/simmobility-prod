//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * \file MT_Message.hpp
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

enum PublicTransitMessage
{
	BOARD_BUS = 6000000,
	ALIGHT_BUS,
	BUS_ARRIVAL,
	BUS_DEPARTURE,
	STORE_BUS_ARRIVAL,
	STORE_PERSON_WAITING,
	STORE_WAITING_PERSON_COUNT,
	STORE_PERSON_TRAVEL_TIME
};

/**
 * Message holding a pointer to BusStop
 */
class BusStopMessage : public messaging::Message {
public:
	BusStopMessage(const BusStop* stop):nextStop(stop){}
	virtual ~BusStopMessage() {}
	const BusStop* nextStop;
};

/**
 * Message holding a pointer to busDriver
 */
class BusDriverMessage : public messaging::Message {
public:
	BusDriverMessage(BusDriver* busDriver):busDriver(busDriver) {}
	virtual ~BusDriverMessage() {}
	BusDriver* busDriver;
};
}
}
