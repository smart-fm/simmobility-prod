/*
 * MT_Statistics.cpp
 *
 *  Created on: 13 Jun, 2014
 *      Author: zhang
 */

#include "entities/MT_Statistics.hpp"
#include "algorithm"

namespace sim_mob {
namespace medium {


MT_Statistics* MT_Statistics::instance(nullptr);

MT_Statistics* MT_Statistics::GetInstance()
{
	if (!instance)
	{
		instance = new MT_Statistics();
	}
	return instance;
}

MT_Statistics::MT_Statistics() : MessageHandler(0) {
	// TODO Auto-generated constructor stub

}

MT_Statistics::~MT_Statistics() {

	std::map<std::string, journeyTimeStats*>::iterator it =
			busJourneyTime.begin();
	for (; it != busJourneyTime.end(); it++) {
		safe_delete_item(it->second);
	}

	std::map<std::string, waittingTimeStats*>::iterator itTm =
			personWaitingTime.begin();
	for (; itTm != personWaitingTime.end(); itTm++) {
		safe_delete_item(itTm->second);
	}
}

void MT_Statistics::HandleMessage(Message::MessageType type,
		const Message& message) {
	switch (type) {
	case STORE_BUS_ARRIVAL: {
		const BusArrivalTimeMessage& msg = MSG_CAST(BusArrivalTimeMessage, message);
		journeyTimeStats* stat = nullptr;
		std::map<std::string, journeyTimeStats*>::iterator it =
				busJourneyTime.find(msg.busStopNo);
		if (it != busJourneyTime.end()) {
			stat = it->second;
		} else {
			stat = new journeyTimeStats();
			busJourneyTime[msg.busStopNo] = stat;
		}

		if (stat) {
			stat->setArrivalTime(msg.busLine, msg.busTrip, msg.sequenceNo,
					msg.arrivalTime);
		}

		break;
	}
	case STORE_PERSON_WAITING: {
		const PersonWaitingTimeMessage& msg = MSG_CAST(PersonWaitingTimeMessage,
				message);
		waittingTimeStats* stat = nullptr;
		std::map<std::string, waittingTimeStats*>::iterator it =
				personWaitingTime.find(msg.busStopNo);
		if (it != personWaitingTime.end()) {
			stat = it->second;
		} else {
			stat = new waittingTimeStats();
			personWaitingTime[msg.busStopNo] = stat;
		}

		if (stat) {
			stat->setWaitingTime(msg.personId, msg.waitingTime);
		}

		break;
	}
	default:
		break;
	}
}

void MT_Statistics::PrintStatistics()
{
	std::map<std::string, journeyTimeStats*>::iterator it =
			busJourneyTime.begin();
	for (; it != busJourneyTime.end(); it++) {
		journeyTimeStats* stat = it->second;
		Print() << "At bus stop " << it->first << std::endl;
		std::vector<busArrivalTimeStats>& busArrivalTm =
				stat->getArrivalTime();
		std::vector<busArrivalTimeStats>::iterator itArrivalTm =
				busArrivalTm.begin();
		//std::sort(busArrivalTm.begin(), busArrivalTm.end());
		while (itArrivalTm != busArrivalTm.end()) {
			Print() << itArrivalTm->busLine << "\t";
			Print() << itArrivalTm->tripId << "\t";
			Print() << itArrivalTm->arrivalTime << "\t";
			Print() << std::endl;
			itArrivalTm++;
		}
	}
}

void journeyTimeStats::setArrivalTime(const std::string& busLine,
		const std::string& tripId, unsigned int sequenceNo,
		const std::string& arrivalTime) {
	busArrivalTimeStats stat;
	stat.busLine = busLine;
	stat.tripId = tripId;
	stat.sequenceNo = sequenceNo;
	stat.arrivalTime = arrivalTime;
	busArrivalTimeList.push_back(stat);
}

bool busArrivalTimeStats::operator<(const busArrivalTimeStats& rhs) const {
	if(busLine < rhs.busLine){
		return true;
	}
	else if(busLine > rhs.busLine){
		return false;
	}
	else{
		return tripId < rhs.tripId;
	}
}
}
}
