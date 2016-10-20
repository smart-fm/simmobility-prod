//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "message/Message.hpp"
#include "entities/Person_ST.hpp"
#include "geospatial/network/RoadSegment.hpp"

namespace sim_mob
{

class BusStop;
class BusDriver;

const messaging::Message::MessageType MSG_WAITING_PERSON_ARRIVAL = 1000000;
const messaging::Message::MessageType MSG_WAKEUP_MRT_PAX = 1000001;
const messaging::Message::MessageType MSG_WAKEUP_BUS_PAX = 1000002;
const messaging::Message::MessageType MSG_WAKEUP_WAITING_PERSON = 1000003;
const messaging::Message::MessageType MSG_ATTEMPT_BOARD_BUS = 1000004;
const messaging::Message::MessageType MSG_BOARD_BUS_SUCCESS = 1000005;
const messaging::Message::MessageType MSG_BOARD_BUS_FAIL = 1000006;
const messaging::Message::MessageType MSG_ALIGHT_BUS = 1000007;

/**
 * Message to wrap a Person
 */
class PersonMessage: public messaging::Message
{
public:
	Person_ST* person;

	PersonMessage(Person_ST *inPerson) :
	person(inPerson)
	{
	}

	virtual ~PersonMessage()
	{
	}
} ;

/**
 * Subclass wraps a bus stop into message so as to make alighting decision.
 * This is to allow it to function as an message callback parameter.
 */
class ArrivalAtStopMessage: public messaging::Message
{
public:
	Person_ST *waitingPerson;

	ArrivalAtStopMessage(Person_ST *person) :
	waitingPerson(person)
	{
	}

	virtual ~ArrivalAtStopMessage()
	{
	}
};

/**
 * Message holding a pointer to BusStop
 */
class BusStopMessage: public messaging::Message
{
public:
	const BusStop *nextStop;
	BusDriver *busDriver;
	
	BusStopMessage(const BusStop *stop, BusDriver *busDriver) :
	nextStop(stop), busDriver(busDriver)
	{
	}

	virtual ~BusStopMessage()
	{
	}
} ;

/**
 * Message holding a pointer to BusDriver
 */
class BusDriverMessage: public messaging::Message
{
public:
	const BusDriver *busDriver;
	
	BusDriverMessage(const BusDriver *busDriver) :
	busDriver(busDriver)
	{
	}

	virtual ~BusDriverMessage()
	{
	}
} ;

}
