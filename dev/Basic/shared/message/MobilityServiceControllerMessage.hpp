#pragma once

#include "Message.hpp"
#include <boost/ptr_container/ptr_vector.hpp>
#include "entities/Person.hpp"
#include <stdexcept>


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



/*
struct TripRequest
{
	TripRequest():currTick(timeslice(0,0)){};
	~TripRequest(){};
	TripRequest(const TripRequest& r):userId(r.userId),currTick(r.currTick),startNodeId(r.startNodeId),
			destinationNodeId(r.destinationNodeId),extraTripTimeThreshold(r.extraTripTimeThreshold)
	{};
	std::string userId;
	timeslice currTick;
	//TODO: to enhance performance, instead of storing here node ids,
	// we could directly store Node*, to avoid continuous access to the
	// RoadNetwork::getInstance()->getMapOfIdvsNodes()
	// For example, in SharedController::computeSchedules() we make a search into
	// that map, many times, redundantly and uselessly.
	unsigned int startNodeId;
	unsigned int destinationNodeId;
	unsigned int extraTripTimeThreshold;
};
*/

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
	TripRequestMessage():timeOfRequest(timeslice(0,0)),userId("no-id"),startNodeId(0),
		destinationNodeId(0), extraTripTimeThreshold(0){};

	TripRequestMessage(const TripRequestMessage& r) :
		timeOfRequest(r.timeOfRequest),
			userId(r.userId), startNodeId(r.startNodeId),
			destinationNodeId(r.destinationNodeId),
			extraTripTimeThreshold(r.extraTripTimeThreshold)
		{
		};



	TripRequestMessage(const timeslice& ct, const std::string& p,
		const unsigned int& sn, const unsigned int& dn,
		const unsigned int& threshold) : timeOfRequest(ct), userId(p),
			startNodeId(sn), destinationNodeId(dn),
			extraTripTimeThreshold(threshold)
	{
	};


	~TripRequestMessage()
	{
	}

	bool operator==(const TripRequestMessage& other) const;
	bool operator!=(const TripRequestMessage& other) const;
	bool operator<(const TripRequestMessage& other) const;

	timeslice timeOfRequest;
	std::string userId;
	unsigned int startNodeId;
	unsigned int destinationNodeId;
	unsigned int extraTripTimeThreshold;
};

enum ScheduleItemType{INVALID,PICKUP, DROPOFF,CRUISE};

struct ScheduleItem
{
	ScheduleItem(const ScheduleItemType scheduleItemType_, const TripRequestMessage tripRequest_)
		: scheduleItemType(scheduleItemType_),tripRequest(tripRequest_),tazToCruiseTo(0)
	{
#ifndef NDEBUG
		if (scheduleItemType!= ScheduleItemType::PICKUP && scheduleItemType!= ScheduleItemType::DROPOFF)
			throw std::runtime_error("Only PICKUP or DROPOFF is admitted here");
#endif

	};

	ScheduleItem(const ScheduleItemType scheduleItemType_, const unsigned tazToCruiseTo_)
		:scheduleItemType(scheduleItemType_),tazToCruiseTo(tazToCruiseTo_),tripRequest()
	{
#ifndef NDEBUG
		if (scheduleItemType!= ScheduleItemType::CRUISE)
			throw std::runtime_error("Only CRUISE is admitted here");
#endif
	};

	ScheduleItem(const ScheduleItem& other):scheduleItemType(other.scheduleItemType),tripRequest(other.tripRequest),tazToCruiseTo(other.tazToCruiseTo){};

	bool operator<(const ScheduleItem& other) const;

	ScheduleItemType scheduleItemType;

	TripRequestMessage tripRequest;

	unsigned tazToCruiseTo;
};

//TODO: It would be more elegant using std::variant, available from c++17
typedef std::vector<ScheduleItem> Schedule;

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
	SchedulePropositionMessage(const timeslice currTick_, Schedule schedule_):
		currTick(currTick_), schedule(schedule_){};

	const Schedule& getSchedule() const;
	const timeslice currTick;

private:
	Schedule schedule;
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

class ScheduleException: public std::runtime_error {
public:
	ScheduleException(const std::string& xmsg) : std::runtime_error (xmsg) {} ;
	virtual ~ScheduleException(){};
};

}


std::ostream& operator<<(std::ostream& strm, const sim_mob::TripRequestMessage& request);
