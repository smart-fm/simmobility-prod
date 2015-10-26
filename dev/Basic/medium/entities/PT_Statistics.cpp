/*
 * PT_Statistics.cpp
 *
 *  Created on: 13 Jun, 2014
 *      Author: zhang huai peng
 */

#include "entities/PT_Statistics.hpp"
#include <cstdio>
#include <fstream>
#include "config/MT_Config.hpp"
#include "util/LangHelpers.hpp"

namespace sim_mob
{
namespace medium
{
PT_Statistics* PT_Statistics::instance(nullptr);

PT_Statistics* PT_Statistics::getInstance()
{
	if (!instance)
	{
		instance = new PT_Statistics();
	}
	return instance;
}

void PT_Statistics::resetInstance()
{
	safe_delete_item(instance);
	instance = nullptr;
}

PT_Statistics::PT_Statistics() : MessageHandler(0) {}

PT_Statistics::~PT_Statistics() {}

void PT_Statistics::HandleMessage(Message::MessageType type, const Message& message)
{
	switch (type)
	{
	case STORE_BUS_ARRIVAL:
	{
		const BusArrivalTimeMessage& msg = MSG_CAST(BusArrivalTimeMessage, message);
		busJourneyTimes.push_back(msg.busArrivalInfo);
		break;
	}
	case STORE_PERSON_WAITING:
	{
		const PersonWaitingTimeMessage& msg = MSG_CAST(PersonWaitingTimeMessage, message);
		char key[50];
		sprintf(key, "%u,%s", msg.personWaitingTime.personId, msg.personWaitingTime.busStopNo.c_str());
		personWaitingTimes[std::string(key)] = msg.personWaitingTime;
		break;
	}
	case STORE_PERSON_TRAVEL_TIME:
	{
		const PersonTravelTimeMessage& msg = MSG_CAST(PersonTravelTimeMessage, message);
		personTravelTimes.push_back(msg.personTravelTime);
		break;
	}
	case STORE_WAITING_PERSON_COUNT:
	{
		const WaitingCountMessage& msg = MSG_CAST(WaitingCountMessage, message);
		waitingCounts.push_back(msg.waitingCnt);
		break;
	}
	default:
	{
		break;
	}
	}
}

void PT_Statistics::storeStatistics()
{
	const MT_Config& mtcfg = MT_Config::getInstance();
	std::string journeyStatsFilename = mtcfg.getJourneyTimeStatsFilename();
	if (!journeyStatsFilename.empty())
	{
		std::ofstream outputFile(journeyStatsFilename.c_str());
		if (outputFile.is_open())
		{
			std::vector<BusArrivalTime>::const_iterator it = busJourneyTimes.begin();
			for (; it != busJourneyTimes.end(); it++)
			{
				outputFile << it->getCSV();
			}
			outputFile.close();
		}
	}
	busJourneyTimes.clear();

	std::string waitingTimeStatsFilename = mtcfg.getWaitingTimeStatsFilename();
	if (!waitingTimeStatsFilename.empty())
	{
		std::ofstream outputFile(waitingTimeStatsFilename.c_str());
		if (outputFile.is_open())
		{
			std::map<std::string, PersonWaitingTime>::const_iterator itWaitingTime = personWaitingTimes.begin();
			for (; itWaitingTime != personWaitingTimes.end(); itWaitingTime++)
			{
				outputFile << itWaitingTime->second.getCSV();
					outputFile << itArrivalTm->pctOccupancy << ",";
			}
			outputFile.close();
		}
	}
	personWaitingTimes.clear();

	std::string waitingCountsFilename = mtcfg.getWaitingCountStatsFilename();
	if (waitingCountsFilename.size() > 0)
	{
		std::ofstream outputFile(waitingCountsFilename.c_str());
		if (outputFile.is_open())
		{
			std::vector<WaitingCount>::const_iterator itWaitingCnt = waitingCounts.begin();
			for (; itWaitingCnt!=waitingCounts.end(); itWaitingCnt++)
			{
				outputFile << itWaitingCnt->getCSV();
					outputFile << itWaitingTime->second.busLines << ",";
			}
			outputFile.close();
		}
	}
	waitingCounts.clear();

	std::string travelTimeFilename = mtcfg.getTravelTimeStatsFilename();
	if (travelTimeFilename.size() > 0)
	{
		std::ofstream outputFile(travelTimeFilename.c_str());
		if (outputFile.is_open())
		{
			std::vector<PersonTravelTime>::const_iterator itPersonTravel = personTravelTimes.begin();
			for (; itPersonTravel != personTravelTimes.end(); itPersonTravel++)
			{
				outputFile << itPersonTravel->getCSV();
			}
			outputFile.close();
		}
	}
	personTravelTimes.clear();
}

std::string PersonWaitingTime::getCSV() const
{
	char csvArray[100];
	sprintf(csvArray, "%u,%s,%s,%s,%.2f,%u\n",
			personId,
			busStopNo.c_str(),
			busLines.c_str(),
			currentTime.c_str(),
			waitingTime,
			deniedBoardingCount);
	return std::string(csvArray);
}

std::string BusArrivalTime::getCSV() const
{
	char csvArray[100];
	sprintf(csvArray, "%s,%s,%s,%3.2f,%u,%s,%s\n",
			busStopNo.c_str(),
			busLine.c_str(),
			tripId.c_str(),
			pctOccupancy,
			sequenceNo,
			arrivalTime.c_str(),
			dwellTime.c_str());
	return std::string(csvArray);
}

std::string PersonTravelTime::getCSV() const
{
	char csvArray[100];
	sprintf(csvArray, "%u,%s,%s,%s,%s,%s,%s,%s,%s,%.2f\n",
			personId,
			tripStartPoint.c_str(),
			tripEndPoint.c_str(),
			subStartPoint.c_str(),
			subEndPoint.c_str(),
			subStartType.c_str(),
			subEndType.c_str(),
			mode.c_str(),
			arrivalTime.c_str(),
			travelTime);
	return std::string(csvArray);
}

std::string WaitingCount::getCSV() const
{
	char csvArray[100];
	sprintf(csvArray, "%s,%s,%u\n",
			busStopNo.c_str(),
			currTime.c_str(),
			count);
	return std::string(csvArray);
}

}
}
