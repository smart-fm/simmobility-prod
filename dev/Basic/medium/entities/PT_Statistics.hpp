//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * BusStopAgent.hpp
 *
 *  Created on: 16 June, 2014
 *      Author: zhang huai peng
 */

#pragma once

#include "message/MT_Message.hpp"
#include "entities/misc/BusTrip.hpp"

namespace sim_mob {
namespace medium {
using namespace messaging;

struct PersonWaitingInfo
{
	std::string waitingTime;
	unsigned int failedBoardingTime;
};

class WaitingTimeStats
{
public:
	void setWaitingTime(const std::string& personId, const std::string& waitingTime, unsigned int failedBoardingTimes){
		if(waitingTimeList.find(personId)==waitingTimeList.end()){
			waitingTimeList.insert(std::make_pair(personId, PersonWaitingInfo() ));
		}
		waitingTimeList[personId].waitingTime = waitingTime;
		waitingTimeList[personId].failedBoardingTime = failedBoardingTimes;
	}

	const std::map<std::string, PersonWaitingInfo>& getWaitingTimeList() const {
		return waitingTimeList;
	}
private:
	std::map<std::string, PersonWaitingInfo> waitingTimeList;
};

struct BusArrivalTime {
	std::string busLine;
	std::string tripId;
	unsigned int sequenceNo;
	std::string arrivalTime;
	std::string dwellTime;
	double pctOccupancy;
	bool operator<(const BusArrivalTime& rhs) const;
};

class JourneyTimeStats {
public:
	void setArrivalTime(const std::string& busLine, const std::string& tripId,
			unsigned int sequenceNo, const std::string& arrivalTime, const std::string& dwellTime, double pctOccupancy);

	const std::vector<BusArrivalTime>& getArrivalTime() const{
		return busArrivalTimeList;
	}
private:
	std::vector<BusArrivalTime> busArrivalTimeList;
};

struct PersonTravelTime {
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
};

class PersonTravelStats {
public:
	void setPersonTravelTime(const std::string& personId, const std::string& tripStartPoint,
			const std::string& tripEndPoint,
			const std::string& subStartPoint, const std::string& subEndPoint,
			const std::string& subStartType, const std::string& subEndType,
			const std::string& mode, const std::string& service,
			const std::string& arrivalTime, const std::string& travelTime);
	const std::vector<PersonTravelTime>& getPersonTravelTime() const {
		return PersonTravelTimeList;
	}
private:
	std::vector<PersonTravelTime> PersonTravelTimeList;
};

struct WaitingAmountStats {
	std::string timeSlice;
	std::string waitingAmount;
};

class PT_Statistics : public messaging::MessageHandler {
public:
	PT_Statistics();
	virtual ~PT_Statistics();
	static PT_Statistics* GetInstance();

    virtual void HandleMessage(Message::MessageType type, const Message& message);

    /**
     * print out statistics information
     */
    void PrintStatistics();

    /**
     * store the statistics to file
     */
    void StoreStatistics();
private:
    /**store stop to stop journey time. bus stop No. is key */
    std::map<std::string, JourneyTimeStats*> busJourneyTime;
    /**store waiting time at bus stop. bus stop No. is key*/
    std::map<std::string, WaitingTimeStats*> personWaitingTime;
    /**store waiting number at each bus stop */
    std::map<std::string, std::vector<WaitingAmountStats> > waitingAmounts;
    /**store travel time*/
    std::map<std::string, PersonTravelStats*> personTravelTimes;
    static PT_Statistics* instance;
 };

}
}
