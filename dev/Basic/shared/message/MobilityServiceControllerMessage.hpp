#pragma once

#include "Message.hpp"

namespace sim_mob
{

enum MobilityServiceControllerMessage
{
	MSG_DRIVER_SUBSCRIBE = 7000000,
	MSG_DRIVER_UNSUBSCRIBE,
	MSG_DRIVER_AVAILABLE,
	MSG_TRIP_REQUEST,
	MSG_SCHEDULE_PROPOSITION,
	MSG_SCHEDULE_PROPOSITION_REPLY
};

/**
 * Message to subscribe a driver
 */
class DriverSubscribeMessage: public messaging::Message
{
public:
	DriverSubscribeMessage(Person* p) : person(p)
	{
	}

	virtual ~DriverSubscribeMessage()
	{
	}

	Person* person;
};

/**
 * Message to unsubscribe a driver
 */
class DriverUnsubscribeMessage: public messaging::Message
{
public:
	DriverUnsubscribeMessage(Person* p) : person(p)
	{
	}

	virtual ~DriverUnsubscribeMessage()
	{
	}

	Person* person;
};

/**
 * Message to state that a driver is available
 */
class DriverAvailableMessage: public messaging::Message
{
public:
	DriverAvailableMessage(Person* p) : person(p)
	{
	}

	virtual ~DriverAvailableMessage()
	{
	}

	Person* person;
};

/**
 * Message to request a trip
 */
class TripRequestMessage: public messaging::Message
{
public:
	TripRequestMessage(timeslice ct, const std::string& p,
		const unsigned int sn, const unsigned int dn,
		const unsigned int threshold) : currTick(ct), personId(p),
			startNodeId(sn), destinationNodeId(dn),
			extraTripTimeThreshold(threshold)
	{
	}

	virtual ~TripRequestMessage()
	{
	}

	const timeslice currTick;
	const std::string personId;
	const unsigned int startNodeId;
	const unsigned int destinationNodeId;
	const unsigned int extraTripTimeThreshold;
};

/**
 * Message to propose a trip to a driver
 */
class SchedulePropositionMessage: public messaging::Message
{
public:
	SchedulePropositionMessage(timeslice ct, const std::string& person,
		const unsigned int sn, const unsigned int dn,
		const unsigned int threshold) : currTick(ct), personId(person),
			startNodeId(sn), destinationNodeId(dn),
			extraTripTimeThreshold(threshold)
	{
	}
	virtual ~SchedulePropositionMessage()
	{
	}

	const timeslice currTick;
	const std::string personId;
	const unsigned int startNodeId;
	const unsigned int destinationNodeId;
	const unsigned int extraTripTimeThreshold;
};

/**
 * Message to respond to a trip proposition
 */
class SchedulePropositionReplyMessage: public messaging::Message
{
public:
	SchedulePropositionReplyMessage(timeslice ct, const std::string& p,
		Person* t, const unsigned int sn, const unsigned int dn,
		const unsigned int threshold, const bool s) : currTick(ct),
			personId(p), driver(t), startNodeId(sn), destinationNodeId(dn),
			extraTripTimeThreshold(threshold), success(s)
	{
	}

	virtual ~SchedulePropositionReplyMessage()
	{
	}

	const timeslice currTick;
	const bool success;
	const std::string personId;
	Person* driver;
	const unsigned int startNodeId;
	const unsigned int destinationNodeId;
	const unsigned int extraTripTimeThreshold;
};

}


