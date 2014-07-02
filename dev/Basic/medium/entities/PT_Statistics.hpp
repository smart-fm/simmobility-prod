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

struct personWaitingInfo{
	std::string waitingTime;
	unsigned int failedBoardingTime;
};

class waittingTimeStats{
public:
	void setWaitingTime(const std::string& personId, const std::string& waitingTime, unsigned int failedBoardingTimes){
		if(waitingTimeList.find(personId)==waitingTimeList.end()){
			waitingTimeList.insert(std::make_pair(personId, personWaitingInfo() ));
		}
		waitingTimeList[personId].waitingTime = waitingTime;
		waitingTimeList[personId].failedBoardingTime = failedBoardingTimes;
	}

	const std::map<std::string, personWaitingInfo>& getWaitingTimeList() const {
		return waitingTimeList;
	}
private:
	std::map<std::string, personWaitingInfo> waitingTimeList;
};

struct busArrivalTime {
	std::string busLine;
	std::string tripId;
	unsigned int sequenceNo;
	std::string arrivalTime;
	std::string dwellTime;
	bool operator<(const busArrivalTime& rhs) const;
};

class journeyTimeStats {
public:
	void setArrivalTime(const std::string& busLine, const std::string& tripId,
			unsigned int sequenceNo, const std::string& arrivalTime, const std::string& dwellTime);

	const std::vector<busArrivalTime>& getArrivalTime() const{
		return busArrivalTimeList;
	}
private:
	std::vector<busArrivalTime> busArrivalTimeList;
};

struct waitingAmountStats {
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
    std::map<std::string, journeyTimeStats*> busJourneyTime;
    /**store waiting time at bus stop. bus stop No. is key*/
    std::map<std::string, waittingTimeStats*> personWaitingTime;
    /**store waiting number at each bus stop */
    std::map<std::string, std::vector<waitingAmountStats> > waitingAmounts;
    static PT_Statistics* instance;
 };

}
}
