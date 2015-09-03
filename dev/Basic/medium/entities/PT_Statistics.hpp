//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "message/MT_Message.hpp"
#include "entities/misc/BusTrip.hpp"

namespace sim_mob
{
namespace medium
{
using namespace messaging;

struct PersonWaitingTime
{
	std::string personId;
	std::string busStopNo;
	std::string currentTime;
	std::string waitingTime;
	std::string busLines;
	unsigned int failedBoardingCount;

	std::string getCSV() const;
};

/**
 * Message to transfer person waiting time at bus stop
 */
class PersonWaitingTimeMessage: public messaging::Message
{
public:
	PersonWaitingTimeMessage(const PersonWaitingTime& personWaitingInfo) : personWaitingTime(personWaitingInfo)	{}
	virtual ~PersonWaitingTimeMessage() {}
	PersonWaitingTime personWaitingTime;
};

struct BusArrivalTime
{
	std::string busStopNo;
	std::string busLine;
	std::string tripId;
	unsigned int sequenceNo;
	std::string arrivalTime;
	std::string dwellTime;
	double pctOccupancy; //percentage

	bool operator<(const BusArrivalTime& rhs) const;
	std::string getCSV() const;
};

/**
 * Message to transfer bus arrival time at bus stop
 */
class BusArrivalTimeMessage : public messaging::Message
{
public:
	BusArrivalTimeMessage(const BusArrivalTime& busArrivalInfo) : busArrivalInfo(busArrivalInfo) {}
	virtual ~BusArrivalTimeMessage() {}
	BusArrivalTime busArrivalInfo;
};

struct PersonTravelTime
{
	std::string personId;
	std::string tripStartPoint;
	std::string tripEndPoint;
	std::string subStartPoint;
	std::string subEndPoint;
	std::string subStartType;
	std::string subEndType;
	std::string mode;
	std::string service;
	std::string arrivalTime;
	std::string travelTime;

	std::string getCSV() const;
};

/**
 * Message for person travel time
 */
class PersonTravelTimeMessage: public messaging::Message
{
public:
	PersonTravelTimeMessage(const PersonTravelTime& personTravelTime) : personTravelTime(personTravelTime) {}
	virtual ~PersonTravelTimeMessage() {}

	PersonTravelTime personTravelTime;
};

struct WaitingCount
{
	std::string timeSlice;
	std::string busStopNo;
	unsigned int count;

	std::string getCSV() const;
};

/**
 * waiting count message
 */
class WaitingCountMessage: public messaging::Message
{
public:
	WaitingCountMessage(const WaitingCount& cnt) : waitingCnt(cnt) {}
	virtual ~WaitingCountMessage() {}
	WaitingCount waitingCnt;
};

class PT_Statistics : public messaging::MessageHandler
{
public:
	virtual ~PT_Statistics();

	static PT_Statistics* getInstance();
	static void resetInstance();

	virtual void HandleMessage(Message::MessageType type, const Message& message);

	/**
	 * store the statistics to file
	 */
	void storeStatistics();

private:
	PT_Statistics();

	/**store stop to stop journey time. bus stop No. is key */
	std::vector<BusArrivalTime> busJourneyTimes;

	/**store for waiting time at bus stop*/
	std::vector<PersonWaitingTime> personWaitingTimes;

	/**store for number of persons waiting at each bus stop */
	std::vector<WaitingCount> waitingCounts;

	/**travel time store*/
	std::vector<PersonTravelTime> personTravelTimes;

	static PT_Statistics* instance;
};

}
}
