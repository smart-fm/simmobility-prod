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

class waittingTimeStats{
public:
	void setWaitingTime(const std::string& personId, const std::string& waitingTime){
		waitingTimeList[personId] = waitingTime;
	}
private:
	std::map<std::string, std::string> waitingTimeList;
};

struct busArrivalTimeStats {
	std::string busLine;
	std::string tripId;
	unsigned int sequenceNo;
	std::string arrivalTime;
	bool operator<(const busArrivalTimeStats& rhs) const;
};

class journeyTimeStats {
public:
	void setArrivalTime(const std::string& busLine, const std::string& tripId,
			unsigned int sequenceNo, const std::string& arrivalTime);

	std::vector<busArrivalTimeStats>& getArrivalTime(){
		return busArrivalTimeList;
	}
private:
	std::vector<busArrivalTimeStats> busArrivalTimeList;
};

class MT_Statistics : public messaging::MessageHandler {
public:
	MT_Statistics();
	virtual ~MT_Statistics();
	static MT_Statistics* GetInstance();

    virtual void HandleMessage(Message::MessageType type, const Message& message);

    /**
     * print out statistics information
     */
    void PrintStatistics();
private:
    /**store stop to stop journey time. bus stop No. is key */
    std::map<std::string, journeyTimeStats*> busJourneyTime;
    /**store waiting time at bus stop. bus stop No. is key*/
    std::map<std::string, waittingTimeStats*> personWaitingTime;
    static MT_Statistics* instance;
 };

}
}
