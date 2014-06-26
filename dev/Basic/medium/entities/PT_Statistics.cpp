/*
 * PT_Statistics.cpp
 *
 *  Created on: 13 Jun, 2014
 *      Author: zhang huai peng
 */

#include "entities/PT_Statistics.hpp"
#include "algorithm"
#include "config/MT_Config.hpp"
#include "iostream"
#include "fstream"

namespace sim_mob {
namespace medium {


PT_Statistics* PT_Statistics::instance(nullptr);

PT_Statistics* PT_Statistics::GetInstance()
{
	if (!instance)
	{
		instance = new PT_Statistics();
	}
	return instance;
}

PT_Statistics::PT_Statistics() : MessageHandler(0) {
	// TODO Auto-generated constructor stub

}

PT_Statistics::~PT_Statistics() {

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

void PT_Statistics::HandleMessage(Message::MessageType type,
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
			stat->setWaitingTime(msg.personId, msg.waitingTime, msg.failedBoardingTimes);
		}

		break;
	}
	default:
		break;
	}
}

void PT_Statistics::PrintStatistics() {
	Print()
			<< "###################      PT_Statistics      ####################"
			<< std::endl;
	Print() << "\n" << std::endl;
	std::map<std::string, journeyTimeStats*>::iterator it =
			busJourneyTime.begin();
	for (; it != busJourneyTime.end(); it++) {
		journeyTimeStats* stat = it->second;
		std::string stopNo = it->first;
		Print() << "Bus arrival time at bus stop " << stopNo << std::endl;
		const std::vector<busArrivalTime>& busArrivalTm =
				stat->getArrivalTime();
		std::vector<busArrivalTime>::const_iterator itArrivalTm =
				busArrivalTm.begin();
		//std::sort(busArrivalTm.begin(), busArrivalTm.end());
		while (itArrivalTm != busArrivalTm.end()) {
			Print() << "Stop: " << stopNo << "\t\t";
			Print() << "Line: " << itArrivalTm->busLine << "\t";
			Print() << "Trip: " << std::setfill('0') << std::setw(3)
					<< itArrivalTm->tripId << "\t";
			Print() << "Arriving: " << itArrivalTm->arrivalTime << "\t";
			Print() << std::endl;
			itArrivalTm++;
		}
	}

	waittingTimeStats* stat = nullptr;
	std::map<std::string, waittingTimeStats*>::iterator itWaitingPeople =
			personWaitingTime.begin();
	for (; itWaitingPeople != personWaitingTime.end(); itWaitingPeople++) {
		stat = itWaitingPeople->second;
		std::string stopNo = itWaitingPeople->first;
		Print() << "people waiting time at bus stop " << stopNo << std::endl;
		const std::map<std::string, personWaitingInfo>& waitingTimeList =
				stat->getWaitingTimeList();
		std::map<std::string, personWaitingInfo>::const_iterator itWaitingTime =
				waitingTimeList.begin();
		while (itWaitingTime != waitingTimeList.end()) {
			Print() << "Stop: " << stopNo << "\t\t";
			Print() << "Id: " << std::setfill('0') << std::setw(5)
					<< itWaitingTime->first << "\t";
			Print() << "Waiting: " << itWaitingTime->second.waitingTime << "\t";
			Print() << "FailedBoarding: "
					<< itWaitingTime->second.failedBoardingTime << "\t";
			Print() << std::endl;
			itWaitingTime++;
		}
	}

	Print() << "#######################################################"
			<< std::endl;
	Print() << "\n" << std::endl;

	StoreStatistics();
}

void PT_Statistics::StoreStatistics() {
	std::string filenameOfJourneyStats =
			MT_Config::GetInstance().getFilenameOfJourneyStats();
	if (filenameOfJourneyStats.size() > 0) {
		std::ofstream outputFile(filenameOfJourneyStats.c_str());
		if (outputFile.is_open()) {
			std::map<std::string, journeyTimeStats*>::iterator it =
					busJourneyTime.begin();
			for (; it != busJourneyTime.end(); it++) {
				journeyTimeStats* stat = it->second;
				std::string stopNo = it->first;
				const std::vector<busArrivalTime>& busArrivalTm =
						stat->getArrivalTime();
				std::vector<busArrivalTime>::const_iterator itArrivalTm =
						busArrivalTm.begin();
				while (itArrivalTm != busArrivalTm.end()) {
					outputFile << stopNo << ",";
					outputFile << itArrivalTm->busLine << ",";
					outputFile << itArrivalTm->tripId << ",";
					outputFile << itArrivalTm->arrivalTime << std::endl;
					itArrivalTm++;
				}
			}
			outputFile.close();
		}
	}

	std::string filenameOfWaitingStats =
			MT_Config::GetInstance().getFilenameOfWaitingStats();
	if (filenameOfWaitingStats.size() > 0) {
		std::ofstream outputFile(filenameOfWaitingStats.c_str());
		if (outputFile.is_open()) {
			waittingTimeStats* stat = nullptr;
			std::map<std::string, waittingTimeStats*>::iterator itWaitingPeople =
					personWaitingTime.begin();
			for (; itWaitingPeople != personWaitingTime.end();
					itWaitingPeople++) {
				stat = itWaitingPeople->second;
				std::string stopNo = itWaitingPeople->first;
				const std::map<std::string, personWaitingInfo>& waitingTimeList =
						stat->getWaitingTimeList();
				std::map<std::string, personWaitingInfo>::const_iterator itWaitingTime =
						waitingTimeList.begin();
				while (itWaitingTime != waitingTimeList.end()) {
					outputFile << stopNo << ",";
					outputFile << itWaitingTime->first << ",";
					outputFile << itWaitingTime->second.waitingTime << ",";
					outputFile << itWaitingTime->second.failedBoardingTime
							<< std::endl;
					itWaitingTime++;
				}
			}
			outputFile.close();
		}
	}
}

void journeyTimeStats::setArrivalTime(const std::string& busLine,
		const std::string& tripId, unsigned int sequenceNo,
		const std::string& arrivalTime) {
	busArrivalTime stat;
	stat.busLine = busLine;
	stat.tripId = tripId;
	stat.sequenceNo = sequenceNo;
	stat.arrivalTime = arrivalTime;
	busArrivalTimeList.push_back(stat);
}

bool busArrivalTime::operator<(const busArrivalTime& rhs) const {
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
