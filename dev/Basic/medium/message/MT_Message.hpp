//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "message/Message.hpp"
#include "entities/Person_MT.hpp"
#include "geospatial/network/RoadSegment.hpp"

namespace sim_mob
{
class BusStop;

namespace medium
{
class BusDriver;

enum ConfluxMessage
{
	MSG_PEDESTRIAN_TRANSFER_REQUEST = 5000000,
	MSG_INSERT_INCIDENT,
	MSG_WAITING_PERSON_ARRIVAL,
	MSG_WAKEUP_STASHED_PERSON,
	MSG_WAKEUP_MRT_PAX,
	MSG_WAKEUP_PEDESTRIAN,
	MSG_WARN_INCIDENT,
	MSG_PERSON_LOAD,
	MSG_PERSON_TRANSFER,
	MSG_TRAVELER_TRANSFER,
	// Vehicle Controller Messages
	MSG_VEHICLE_REQUEST,
	MSG_VEHICLE_ASSIGNMENT
};

enum PublicTransitMessage
{
	BOARD_BUS = 6000000,
	ALIGHT_BUS,
	BUS_ARRIVAL,
	BUS_DEPARTURE
};

enum TaxiMessage
{
	CALL_TAXI = 7000000
};
/**
 * Message holding a pointer to BusStop
 */
class BusStopMessage: public messaging::Message
{
public:
	BusStopMessage(const BusStop* stop) :
			nextStop(stop)
	{
	}
	virtual ~BusStopMessage()
	{
	}
	const BusStop* nextStop;
	std::string busLines;
};
/**
 * Message for calling a taxi
 */
class TaxiCallMessage: public messaging::Message
{
public:
	TaxiCallMessage(const std::string& person, const unsigned int sn, const unsigned int dn) :
			personId(person), startNodeId(sn), destinationNodeId(dn)
	{
	}
	virtual ~TaxiCallMessage()
	{
	}
	const std::string personId;
	const unsigned int startNodeId;
	const unsigned int destinationNodeId;
};
/**
 * Message holding a pointer to busDriver
 */
class BusDriverMessage: public messaging::Message
{
public:
	BusDriverMessage(BusDriver* busDriver) :
			busDriver(busDriver)
	{
	}
	virtual ~BusDriverMessage()
	{
	}
	BusDriver* busDriver;
};

/**
 * Message to wrap a Person
 */
class PersonMessage: public messaging::Message
{
public:
	PersonMessage(Person_MT* inPerson) :
			person(inPerson)
	{
	}

	virtual ~PersonMessage()
	{
	}

	Person_MT* person;
};

/**
 * Message to notify incidents
 */
class InsertIncidentMessage: public messaging::Message
{
public:
	InsertIncidentMessage(const RoadSegment* rs, double newFlowRate) :
			affectedSegment(rs), newFlowRate(newFlowRate)
	{
	}

	virtual ~InsertIncidentMessage()
	{
	}

	const RoadSegment* affectedSegment;
	double newFlowRate;
};

/**
 * Subclass wraps a bus stop into message so as to make alighting decision.
 * This is to allow it to function as an message callback parameter.
 */
class ArrivalAtStopMessage: public messaging::Message
{
public:
	ArrivalAtStopMessage(Person_MT* person) :
			waitingPerson(person)
	{
	}

	virtual ~ArrivalAtStopMessage()
	{
	}

	Person_MT* waitingPerson;
};

/**
 * Message to request a vehicle
 */
class VehicleRequestMessage: public messaging::Message
{
public:
	VehicleRequestMessage(const std::string& p, const unsigned int sn, const unsigned int dn) :
			personId(p), startNodeId(sn), destinationNodeId(dn)
	{
	}

	virtual ~VehicleRequestMessage()
	{
	}

	const std::string personId;
	const unsigned int startNodeId;
	const unsigned int destinationNodeId;
};

/**
 * Message to request a vehicle
 */
class VehicleAssignmentMessage: public messaging::Message
{
public:
	VehicleAssignmentMessage(const bool s, const std::string& p, const std::string& t, const unsigned int sn, const unsigned int dn) :
			success(s), personId(p), taxiDriverId(t), startNodeId(sn), destinationNodeId(dn)
	{
	}

	virtual ~VehicleAssignmentMessage()
	{
	}

	const bool success;
	const std::string personId;
	const std::string taxiDriverId;
	const unsigned int startNodeId;
	const unsigned int destinationNodeId;
};

}
}



