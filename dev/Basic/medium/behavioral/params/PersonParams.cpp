//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PersonParams.hpp"

#include <boost/algorithm/string.hpp>

using namespace std;
using namespace sim_mob;
using namespace medium;

sim_mob::medium::PersonParams::~PersonParams() {
	for(boost::unordered_map<std::string, TimeWindowAvailability*>::iterator i=timeWindowAvailability.begin(); i!=timeWindowAvailability.end(); i++){
		safe_delete_item(i->second);
	}
}

void PersonParams::initTimeWindows() {
	std::stringstream tw;
	for (double i = 3.25; i < 27.0; i = i + 0.5) {
		for (double j = i; j < 27.0; j = j + 0.5) {
			tw << i << "," << j;
			timeWindowAvailability[tw.str()] = new TimeWindowAvailability(i, j); //initialize availability of all time windows to 1
		}
	}
}

void PersonParams::blockTime(std::string& timeWnd) {
	try {
		std::vector<std::string> times;
		boost::split(times, timeWnd, boost::is_any_of(","));
		double startTime = std::atof(times.front().c_str());
		double endTime = std::atof(times.back().c_str());
		blockTime(startTime, endTime);
	}
	catch(std::exception& e) {
		throw std::runtime_error("invalid time window was passed for blocking");
	}
}

void PersonParams::blockTime(double startTime, double endTime) {
	if(startTime <= endTime) {
		for(boost::unordered_map<std::string, TimeWindowAvailability*>::iterator i=timeWindowAvailability.begin(); i!=timeWindowAvailability.end(); i++){
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

int PersonParams::getTimeWindowAvailability(std::string& timeWnd) {
	return timeWindowAvailability.at(timeWnd)->getAvailability();
}
