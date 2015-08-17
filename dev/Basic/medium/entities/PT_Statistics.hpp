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

struct PersonWaitingInfo
{
	std::string currentTime;
	std::string waitingTime;
	std::string busLines;
	unsigned int failedBoardingCount;

	std::string getCSV() const;
};

/**
 * Message to transfer person waiting time at bus stop
 */
class PersonWaitingTimeMessage: public messaging::Message {
public:
	PersonWaitingTimeMessage(const std::string& stopNo, const std::string& personId, const PersonWaitingInfo& personWaitingInfo)
			: busStopNo(stopNo), personId(personId),personWaitingInfo(personWaitingInfo)
	{}
	virtual ~PersonWaitingTimeMessage() {}

	const std::string busStopNo;
	std::string personId;
	PersonWaitingInfo personWaitingInfo;
};

class WaitingTimeStats
{
public:
	void setWaitingTime(const std::string& personId, const PersonWaitingInfo& personWaitingInfo)
	{
		waitingTimeList[personId] = personWaitingInfo;
	}

	const std::map<std::string, PersonWaitingInfo>& getWaitingTimeList() const
	{
		return waitingTimeList;
	}

private:
	std::map<std::string, PersonWaitingInfo> waitingTimeList;
};

struct BusArrivalTime
{
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
	BusArrivalTimeMessage(const std::string& stopNo, const BusArrivalTime& busArrivalInfo) :
		busStopNo(stopNo), busArrivalInfo(busArrivalInfo)
	{}
	virtual ~BusArrivalTimeMessage() {}

	std::string busStopNo;
	BusArrivalTime busArrivalInfo;
};

class JourneyTimeStats
{
public:
	void setArrivalTime(const BusArrivalTime& busArrivalInfo);

	const std::vector<BusArrivalTime>& getArrivalTime() const
	{
		return busArrivalTimeList;
	}
private:
	std::vector<BusArrivalTime> busArrivalTimeList;
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

class PersonTravelStats
{
public:
	void setPersonTravelTime(PersonTravelTime personTravelTime);

	const std::vector<PersonTravelTime>& getPersonTravelTime() const
	{
		return personTravelTimeList;
	}

private:
	std::vector<PersonTravelTime> personTravelTimeList;
};

/**
 * waiting count message
 */
class WaitingCountMessage: public messaging::Message {
public:
	WaitingCountMessage(const std::string& stopNo, const std::string& timeSlice, unsigned int waitingNum) :
			busStopNo(stopNo), timeSlice(timeSlice), waitingNum(waitingNum) {
	}
	virtual ~WaitingCountMessage() {
	}
	std::string timeSlice;
	std::string busStopNo;
	unsigned int waitingNum;
};

struct WaitingCountStats
{
	std::string timeSlice;
	std::string waitingCount;
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
	std::map<std::string, JourneyTimeStats*> busJourneyTime;

	/**store waiting time at bus stop. bus stop No. is key*/
	std::map<std::string, WaitingTimeStats*> personWaitingTime;

	/**store waiting number at each bus stop */
	std::map<std::string, std::vector<WaitingCountStats> > waitingCounts;

	/**store travel time*/
	std::map<std::string, PersonTravelStats*> personTravelTimes;

	static PT_Statistics* instance;
};

}
}
