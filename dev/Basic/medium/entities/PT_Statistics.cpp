/*
 * PT_Statistics.cpp
 *
 *  Created on: 13 Jun, 2014
 *      Author: zhang huai peng
 */

#include "entities/PT_Statistics.hpp"
#include <algorithm>
#include <boost/lexical_cast.hpp>
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

PT_Statistics::PT_Statistics() : MessageHandler(0)
{}

PT_Statistics::~PT_Statistics()
{
	std::map<std::string, JourneyTimeStats*>::iterator it = busJourneyTime.begin();
	for (; it != busJourneyTime.end(); it++) {
		safe_delete_item(it->second);
	}

	std::map<std::string, WaitingTimeStats*>::iterator itTm = personWaitingTime.begin();
	for (; itTm != personWaitingTime.end(); itTm++) {
		safe_delete_item(itTm->second);
	}

	std::map<std::string, PersonTravelStats*>::iterator itTravel = personTravelTimes.begin();
	for (; itTravel != personTravelTimes.end(); itTravel++) {
		safe_delete_item(itTravel->second);
	}
}

void PT_Statistics::HandleMessage(Message::MessageType type, const Message& message)
{
	switch (type)
	{
	case STORE_BUS_ARRIVAL:
	{
		const BusArrivalTimeMessage& msg = MSG_CAST(BusArrivalTimeMessage, message);
		JourneyTimeStats* stat = nullptr;
		std::map<std::string, JourneyTimeStats*>::iterator it = busJourneyTime.find(msg.busStopNo);
		if (it != busJourneyTime.end())
		{
			stat = it->second;
		}
		else
		{
			stat = new JourneyTimeStats();
			busJourneyTime[msg.busStopNo] = stat;
		}
		stat->setArrivalTime(msg.busArrivalInfo);
		break;
	}
	case STORE_PERSON_WAITING:
	{
		const PersonWaitingTimeMessage& msg = MSG_CAST(PersonWaitingTimeMessage, message);
		WaitingTimeStats* stat = nullptr;
		std::map<std::string, WaitingTimeStats*>::iterator it = personWaitingTime.find(msg.busStopNo);
		if (it != personWaitingTime.end())
		{
			stat = it->second;
		}
		else
		{
			stat = new WaitingTimeStats();
			personWaitingTime[msg.busStopNo] = stat;
		}
		stat->setWaitingTime(msg.personId, msg.personWaitingInfo);
		break;
	}
	case STORE_PERSON_TRAVEL_TIME:
	{
		const PersonTravelTimeMessage& msg = MSG_CAST(PersonTravelTimeMessage, message);
		const PersonTravelTime& pTT = msg.personTravelTime;
		PersonTravelStats* stat = nullptr;
		std::map<std::string, PersonTravelStats*>::iterator it = personTravelTimes.find(pTT.personId);
		if (it != personTravelTimes.end())
		{
			stat = it->second;
		}
		else
		{
			stat = new PersonTravelStats();
			personTravelTimes[pTT.personId] = stat;
		}
		stat->setPersonTravelTime(pTT);
		break;
	}
	case STORE_WAITING_PERSON_COUNT:
	{
		const WaitingCountMessage& msg = MSG_CAST(WaitingCountMessage, message);
		std::map<std::string, std::vector<WaitingCountStats> >::iterator it = waitingCounts.find(msg.busStopNo);
		if (it == waitingCounts.end())
		{
			waitingCounts[msg.busStopNo] = std::vector<WaitingCountStats>();
		}
		WaitingCountStats stat;
		stat.timeSlice = msg.timeSlice;
		stat.waitingCount = boost::lexical_cast<std::string>(msg.waitingNum);
		waitingCounts[msg.busStopNo].push_back(stat);
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
	std::string filenameOfJourneyStats = MT_Config::getInstance().getFilenameOfJourneyTimeStats();
	if (!filenameOfJourneyStats.empty())
	{
		std::ofstream outputFile(filenameOfJourneyStats.c_str());
		if (outputFile.is_open())
		{
			std::map<std::string, JourneyTimeStats*>::iterator it = busJourneyTime.begin();
			for (; it != busJourneyTime.end(); it++)
			{
				std::string stopNo = it->first;
				JourneyTimeStats* stat = it->second;
				const std::vector<BusArrivalTime>& busArrivalTm = stat->getArrivalTime();
				std::vector<BusArrivalTime>::const_iterator itArrivalTm = busArrivalTm.begin();
				while (itArrivalTm != busArrivalTm.end())
				{
					outputFile << stopNo << "," << itArrivalTm->getCSV() << std::endl;
					itArrivalTm++;
				}
			}
			outputFile.close();
		}
	}

	std::string filenameOfWaitingStats = MT_Config::getInstance().getFilenameOfWaitingTimeStats();
	if (filenameOfWaitingStats.size() > 0)
	{
		std::ofstream outputFile(filenameOfWaitingStats.c_str());
		if (outputFile.is_open())
		{
			WaitingTimeStats* stat = nullptr;
			std::map<std::string, WaitingTimeStats*>::iterator itWaitingPeople = personWaitingTime.begin();
			for (; itWaitingPeople != personWaitingTime.end(); itWaitingPeople++)
			{
				stat = itWaitingPeople->second;
				std::string stopNo = itWaitingPeople->first;
				const std::map<std::string, PersonWaitingInfo>& waitingTimeList = stat->getWaitingTimeList();
				std::map<std::string, PersonWaitingInfo>::const_iterator itWaitingTime = waitingTimeList.begin();
				while (itWaitingTime != waitingTimeList.end())
				{
					outputFile << itWaitingTime->first << "," << stopNo << "," << itWaitingTime->second.getCSV() << std::endl;
					itWaitingTime++;
				}
			}
			outputFile.close();
		}
	}

	std::string filenameOfWaitingAmount = MT_Config::getInstance().getFilenameOfWaitingAmountStats();
	if (filenameOfWaitingAmount.size() > 0)
	{
		std::ofstream outputFile(filenameOfWaitingAmount.c_str());
		if (outputFile.is_open())
		{
			std::map<std::string, std::vector<WaitingCountStats> >::iterator itWaitingNum = waitingCounts.begin();
			for (; itWaitingNum != waitingCounts.end(); itWaitingNum++)
			{
				std::vector<WaitingCountStats>& stat = itWaitingNum->second;
				std::string stopNo = itWaitingNum->first;
				std::vector<WaitingCountStats>::iterator it = stat.begin();
				while (it != stat.end())
				{
					outputFile << stopNo << "," << (*it).timeSlice << "," << (*it).waitingCount << std::endl;
					it++;
				}
			}
			outputFile.close();
		}
	}

	std::string filenameOfTravelTime = MT_Config::getInstance().getFilenameOfTravelTimeStats();
	if (filenameOfTravelTime.size() > 0)
	{
		std::ofstream outputFile(filenameOfTravelTime.c_str());
		if (outputFile.is_open())
		{
			PersonTravelStats* stat = nullptr;
			std::map<std::string, PersonTravelStats*>::iterator itPersonTravel = personTravelTimes.begin();
			for (; itPersonTravel != personTravelTimes.end(); itPersonTravel++)
			{
				stat = itPersonTravel->second;
				const std::vector<PersonTravelTime>& travelTime = stat->getPersonTravelTime();
				for (std::vector<PersonTravelTime>::const_iterator i = travelTime.begin(); i != travelTime.end(); i++)
				{
					outputFile << (*i).getCSV() << std::endl;
				}
			}
			outputFile.close();
		}
	}
}

void JourneyTimeStats::setArrivalTime(const BusArrivalTime& busArrivalInfo)
{
	busArrivalTimeList.push_back(busArrivalInfo);
}

void PersonTravelStats::setPersonTravelTime(PersonTravelTime personTravelTime)
{
	DailyTime startTime(personTravelTime.arrivalTime);
	if(startTime.getValue()==0 && personTravelTimeList.size()>0)
	{
		DailyTime lastStart(personTravelTimeList.back().arrivalTime);
		DailyTime lastTravel(personTravelTimeList.back().travelTime);
		startTime=DailyTime(lastStart.getValue()+lastTravel.getValue());
		personTravelTime.arrivalTime = startTime.getStrRepr();
	}
	personTravelTimeList.push_back(personTravelTime);
}

bool BusArrivalTime::operator<(const BusArrivalTime& rhs) const
{
	if (busLine < rhs.busLine)
	{
		return true;
	}
	else if (busLine > rhs.busLine)
	{
		return false;
	}
	else
	{
		return tripId < rhs.tripId;
	}
}

std::string PersonWaitingInfo::getCSV() const
{
	char failedBoardStr[21]; // sufficient to hold all numbers up to 64-bits
	sprintf(failedBoardStr, "%u", failedBoardingCount);
	std::string csv = busLines + "," + currentTime + "," + waitingTime + "," + failedBoardStr;
	return csv;
}

std::string BusArrivalTime::getCSV() const
{
	char seqNoStr[21]; // sufficient to hold all numbers up to 64-bits
	sprintf(seqNoStr, "%u", sequenceNo);
	char pctOccupancyStr[21];
	sprintf(pctOccupancyStr, "%3.2f", pctOccupancy);
	std::string csv = busLine + "," + tripId + "," + pctOccupancyStr + "," + seqNoStr + "," + arrivalTime + "," + dwellTime;
	return csv;
}

std::string PersonTravelTime::getCSV() const
{
	std::string csv = personId + "," + tripStartPoint + "," + tripEndPoint + "," + subStartPoint + "," + subEndPoint + "," +
			subStartType + "," + subEndType + "," + mode + "," + arrivalTime + "," + travelTime;
	return csv;
}
}
}
