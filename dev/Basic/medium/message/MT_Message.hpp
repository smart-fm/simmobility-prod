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
class TrainDriver;
class Passenger;
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
	MSG_PERSON_TRANSFER
};

enum PublicTrainsitEvent
{
	EVT_DISRUPTION_STATION=7000000,
	EVT_DISRUPTION_CHANGEROUTE
};
enum PublicTransitMessage
{
	BOARD_BUS = 6000000,
	ALIGHT_BUS,
	BUS_ARRIVAL,
	BUS_DEPARTURE,
	TRAIN_MOVETO_NEXT_PLATFORM,
	TRAIN_ARRIVAL_AT_STARTPOINT,
	TRAIN_ARRIVAL_AT_ENDPOINT,
	TRAIN_MOVE_AT_UTURN_PLATFORM,
	PASSENGER_ARRIVAL_AT_PLATFORM,
	PASSENGER_LEAVE_FRM_PLATFORM

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
 * Message holding a pointer to trainDriver
 */
class TrainDriverMessage: public messaging::Message
{
public:
	TrainDriverMessage(TrainDriver* driver, bool isHigher=false):
		trainDriver(driver)
	{
		if(isHigher){
			priority += 1;
		}
	}
	virtual ~TrainDriverMessage()
	{
	}
	TrainDriver* trainDriver;
};
/**
 * Message holding a pointer to passenger
 */
class TrainPassengerMessage: public messaging::Message
{
public:
	TrainPassengerMessage(Passenger* passenger):trainPassenger(passenger)
	{

	}
	virtual ~TrainPassengerMessage()
	{

	}
	Passenger* trainPassenger;
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

}
}
