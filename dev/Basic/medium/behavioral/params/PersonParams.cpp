//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PersonParams.hpp"

#include <boost/algorithm/string.hpp>
#include <sstream>
#include "logging/Log.hpp"

using namespace std;
using namespace sim_mob;
using namespace medium;

sim_mob::medium::PersonParams::PersonParams()
: personId(""), personTypeId(-1), ageId(-1), isUniversityStudent(-1), studentTypeId(-1), isFemale(-1),
  incomeId(-1), worksAtHome(-1), carOwnNormal(-1), carOwnOffpeak(-1), motorOwn(-1), hasFixedWorkTiming(-1), homeLocation(-1),
fixedWorkLocation(-1), fixedSchoolLocation(-1), stopType(-1), drivingLicence(-1),
hhOnlyAdults(-1), hhOnlyWorkers(-1), hhNumUnder4(-1), hasUnder15(-1), workLogSum(-1), eduLogSum(-1), shopLogSum(-1), otherLogSum(-1)
{
	initTimeWindows();
}

sim_mob::medium::PersonParams::~PersonParams() {
	timeWindowAvailability.clear();
}

void sim_mob::medium::PersonParams::initTimeWindows() {
	if(!timeWindowAvailability.empty()) { timeWindowAvailability.clear(); }
	for (double i=1; i<=48; i++) {
		for (double j=i; j<=48; j++) {
			timeWindowAvailability.push_back(TimeWindowAvailability(i,j,true)); //make all time windows available
		}
	}
}

void sim_mob::medium::PersonParams::blockTime(double startTime, double endTime) {
	if(startTime <= endTime) {
		for(std::vector<TimeWindowAvailability>::iterator i=timeWindowAvailability.begin(); i!=timeWindowAvailability.end(); i++){
			TimeWindowAvailability& twa = (*i);
			double start = twa.getStartTime();
			double end = twa.getEndTime();
			if((start >= startTime && start <= endTime) || (end >= startTime && end <= endTime)) {
				twa.setAvailability(false);
			}
		}
	}
	else {
		std::stringstream errStream;
		errStream << "invalid time window was passed for blocking"
				<< " |start: " << startTime
				<< " |end: " << endTime
				<<std::endl;
		throw std::runtime_error(errStream.str());
	}
}

int PersonParams::getTimeWindowAvailability(size_t timeWnd) const {
	return timeWindowAvailability[timeWnd-1].getAvailability();
}

void sim_mob::medium::PersonParams::print()
{
	std::stringstream printStrm;
	printStrm << personId << ","
			<< personTypeId << ","
			<< ageId << ","
			<< isUniversityStudent << ","
			<< hhOnlyAdults << ","
			<< hhOnlyWorkers << ","
			<< hhNumUnder4 << ","
			<< hasUnder15 << ","
			<< isFemale << ","
			<< incomeId << ","
			<< missingIncome << ","
			<< worksAtHome << ","
			<< carOwn << ","
			<< carOwnNormal << ","
			<< carOwnOffpeak << ","
			<< motorOwn << ","
			<< workLogSum << ","
			<< eduLogSum << ","
			<< shopLogSum << ","
			<< otherLogSum << std::endl;
	Print() << printStrm.str();
}

int sim_mob::medium::SubTourParams::getTimeWindowAvailability(size_t timeWnd) const
{
	return timeWindowAvailability[timeWnd-1].getAvailability();
}

void sim_mob::medium::SubTourParams::initTimeWindows(double startTime, double endTime)
{
	if(!timeWindowAvailability.empty()) { timeWindowAvailability.clear(); }
	size_t index = 0;
	for (double start=1; start<=48; start++)
	{
		for (double end=start; end<=48; end++)
		{
			if(start >= startTime && end <= endTime)
			{
				timeWindowAvailability.push_back(TimeWindowAvailability(start,end,true));
				availabilityBit[index]=1;
			}
			else { timeWindowAvailability.push_back(TimeWindowAvailability(start,end,false)); }
			index++;
		}
	}
}

void sim_mob::medium::SubTourParams::blockTime(double startTime, double endTime)
{
	if(startTime <= endTime)
	{
		size_t index = 0;
		for(std::vector<TimeWindowAvailability>::iterator i=timeWindowAvailability.begin(); i!=timeWindowAvailability.end(); i++, index++)
		{
			TimeWindowAvailability& twa = (*i);
			double start = twa.getStartTime();
			double end = twa.getEndTime();
			if((start >= startTime && start <= endTime) || (end >= startTime && end <= endTime)) {
				twa.setAvailability(false);
				availabilityBit[index] = 0;
			}
		}
	}
	else {
		std::stringstream errStream;
		errStream << "invalid time window was passed for blocking" << "|start: " << startTime << "|end: " << endTime << std::endl;
		throw std::runtime_error(errStream.str());
	}
}

sim_mob::medium::SubTourParams::SubTourParams(const Tour& parentTour)
: subTourPurpose(parentTour.getTourType()), usualLocation(parentTour.isUsualLocation()), tourMode(parentTour.getTourMode()),
  firstOfMultipleTours(parentTour.isFirstTour()), subsequentOfMultipleTours(!parentTour.isFirstTour())
{
	const Stop* primaryStop = parentTour.getPrimaryStop();
	initTimeWindows(primaryStop->getArrivalTime(), primaryStop->getDepartureTime());
}

sim_mob::medium::SubTourParams::~SubTourParams()
{
}

bool sim_mob::medium::SubTourParams::allWindowsUnavailable()
{
	return availabilityBit.none();
}
