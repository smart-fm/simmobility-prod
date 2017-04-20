#pragma once

#include "Message.hpp"

namespace sim_mob
{

enum MobilityServiceControllerMessage
{
	MSG_TRIP_REQUEST = 7000000,
	MSG_TRIP_PROPOSITION,
	MSG_TRIP_PROPOSITION_REPLY
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
class TripPropositionMessage: public messaging::Message
{
public:
	TripPropositionMessage(timeslice ct, const std::string& person,
		const unsigned int sn, const unsigned int dn,
		const unsigned int threshold) : currTick(ct), personId(person),
			startNodeId(sn), destinationNodeId(dn),
			extraTripTimeThreshold(threshold)
	{
	}
	virtual ~TripPropositionMessage()
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
class TripPropositionReplyMessage: public messaging::Message
{
public:
	TripPropositionReplyMessage(timeslice ct, const std::string& p,
		const std::string& t, const unsigned int sn, const unsigned int dn,
		const unsigned int threshold, const bool s) : currTick(ct),
			personId(p), driverId(t), startNodeId(sn), destinationNodeId(dn),
			extraTripTimeThreshold(threshold), success(s)
	{
	}

	virtual ~TripPropositionReplyMessage()
	{
	}

	const timeslice currTick;
	const bool success;
	const std::string personId;
	const std::string driverId;
	const unsigned int startNodeId;
	const unsigned int destinationNodeId;
	const unsigned int extraTripTimeThreshold;
};

}

