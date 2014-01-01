//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PersonParams.hpp"

#include <boost/algorithm/string.hpp>

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
	for(boost::unordered_map<int, TimeWindowAvailability*>::iterator i=timeWindowAvailability.begin(); i!=timeWindowAvailability.end(); i++){
		delete i->second;
	}
	timeWindowAvailability.clear();
}

void PersonParams::initTimeWindows() {
	int index = 1;
	for (double i = 1; i <= 48; i++) {
		for (double j=i; j<=48; j++) {
			timeWindowAvailability[index] = new TimeWindowAvailability(i, j); //initialize availability of all time windows to 1
			index++;
		}
	}
}

void PersonParams::blockTime(double startTime, double endTime) {
	if(startTime <= endTime) {
		for(boost::unordered_map<int, TimeWindowAvailability*>::iterator i=timeWindowAvailability.begin(); i!=timeWindowAvailability.end(); i++){
			double start = i->second->getStartTime();
			double end = i->second->getEndTime();
			if((start >= startTime && start <= endTime) || (end >= startTime && end <= endTime)) {
				i->second->setAvailability(0);
			}
		}
	}
	else {
		throw std::runtime_error("invalid time window was passed for blocking");
	}
}

int PersonParams::getTimeWindowAvailability(int timeWnd) const {
	return timeWindowAvailability.at(timeWnd)->getAvailability();
}
