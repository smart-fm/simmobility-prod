#pragma once

#include "Message.hpp"
#include <boost/ptr_container/ptr_vector.hpp>
#include "entities/Person.hpp"

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

struct TripRequest
{
	const timeslice currTick;
	const std::string userId;
	//TODO: to enhance performance, instead of storing here node ids,
	// we could directly store Node*, to avoid continuous access to the
	// RoadNetwork::getInstance()->getMapOfIdvsNodes()
	// For example, in SharedController::computeSchedules() we make a search into
	// that map, many times, redundantly and uselessly.
	const unsigned int startNodeId;
	const unsigned int destinationNodeId;
	const unsigned int extraTripTimeThreshold;
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

enum ScheduleItemType{PICKUP, DROPOFF};


class ScheduleItem
{
protected:
	ScheduleItemType scheduleItemType;

public:
	ScheduleItem(){};
	virtual void dummy() = 0 ;
	const ScheduleItemType getScheduleItemType() const;
};


class PickUpScheduleItem : public ScheduleItem
{
public:
	PickUpScheduleItem(const TripRequest request_): request(request_)
	{
		scheduleItemType=PICKUP;
	};
	void dummy(){};
	const TripRequest request;
};

class DropOffScheduleItem : public ScheduleItem
{
public:
	DropOffScheduleItem(const TripRequest request_): request(request_)
	{
		scheduleItemType=DROPOFF;
	};

	void dummy(){};
	const TripRequest request;
};

typedef std::queue<ScheduleItem*> Schedule;

/**
 * Message to propose a trip to a driver
 */
/*
class SchedulePropositionMessage: public messaging::Message
{
public:
	SchedulePropositionMessage(timeslice ct, const std::string& person,
		const unsigned int startNode, const unsigned int destinationNode,
		const unsigned int threshold) :
			currTick(ct), personId(person),
			startNodeId(startNode), destinationNodeId(destinationNode),
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
*/

class SchedulePropositionMessage: public messaging::Message
{
public:
	SchedulePropositionMessage(const timeslice currTick_, Schedule* schedule_):
		currTick(currTick_), schedule(schedule_){};

	Schedule* getSchedule() const;
	const timeslice currTick;

private:
	Schedule* schedule;
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


