//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PredayClasses.hpp
 *
 *  Created on: Nov 11, 2013
 *      Author: Harish Loganathan
 */

#pragma once
#include <deque>
#include <string>
#include <vector>

#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>

#include "util/LangHelpers.hpp"

namespace sim_mob {
namespace medium {

/**
 * An encapsulation of a time window and its availability.
 *
 * startTime and endTime are of the format <3-26>.25 or <3-26>.75
 * The startTime must be lesser than or equal to endTime.
 * x.25 represents a time value between x:00 - x:29
 * x.75 represents a time value between x:30 and x:59
 *
 * 3.25 (0300 to 0329 hrs) is considered that start of the day.
 * 23.75 is 1130 to 1159 hrs
 * 24.25 is 1200 to 1229 hrs
 * ... so on
 * 26.75 is 0230 to 0259 hrs
 * 26.75 is considered the end of the day
 *
 * windowString is of the format "startTime,endTime"
 *
 * \author Harish Loganathan
 */

class TimeWindowAvailability {
public:
	TimeWindowAvailability(double startTime, double endTime);

	int getAvailability() const {
		return availability;
	}

	void setAvailability(int availability) {
		this->availability = availability;
	}

	double getEndTime() const {
		return endTime;
	}

	double getStartTime() const {
		return startTime;
	}

private:
	double startTime;
	double endTime;
	std::string windowString;
	int availability;
};

enum StopType {
	WORK, EDUCATION, SHOP, OTHER
};

class Tour;

/**
 * Representation of an Activity for the demand simulator.
 *
 * \author Harish Loganathan
 */
class Stop {
public:
	Stop(StopType stopType, Tour& parentTour, bool primaryActivity, bool firstHalfTour)
	: stopType(stopType), parentTour(parentTour), primaryActivity(primaryActivity), arrivalTime(0), departureTime(0), stopMode(0), stopLocation(0), inFirstHalfTour(firstHalfTour)
	{}
	double getArrivalTime() const {
		return arrivalTime;
	}

	void setArrivalTime(double arrivalTime) {
		this->arrivalTime = arrivalTime;
	}

	double getDepartureTime() const {
		return departureTime;
	}

	void setDepartureTime(double departureTime) {
		this->departureTime = departureTime;
	}

	const Tour& getParentTour() const {
		return parentTour;
	}

	bool isPrimaryActivity() const {
		return primaryActivity;
	}

	void setPrimaryActivity(bool primaryActivity) {
		this->primaryActivity = primaryActivity;
	}

	long getStopLocation() const {
		return stopLocation;
	}

	void setStopLocation(long stopLocation) {
		this->stopLocation = stopLocation;
	}

	int getStopMode() const {
		return stopMode;
	}

	void setStopMode(int stopMode) {
		this->stopMode = stopMode;
	}

	StopType getStopType() const {
		return stopType;
	}

	int getStopTypeID() const {
		switch(stopType) {
		case WORK: return 1;
		case EDUCATION: return 2;
		case SHOP: return 3;
		case OTHER: return 4;
		}
	}

	void setStopType(StopType stopType) {
		this->stopType = stopType;
	}

	/**
	 * Sets the start and end time for the tour.
	 *
	 * @param timeWindow the time window to be allotted for this activity. Expected format is "<start_time>,<end_time>" where start_time and end_time is either <3-26>.25 or <3-26>.75.
	 */
	void allotTime(std::string& timeWindow) {
		try {
			std::vector<std::string> times;
			boost::split(times, timeWindow, boost::is_any_of(","));
			arrivalTime = std::atof(times.front().c_str());
			departureTime = std::atof(times.back().c_str());
		}
		catch(std::exception& e) {
			throw std::runtime_error("invalid time format supplied");
		}
	}

	void allotTime(double arrivalTime, double departureTime) {
		this->arrivalTime = arrivalTime;
		this->departureTime = departureTime;
	}

	bool isInFirstHalfTour() const {
		return inFirstHalfTour;
	}

private:
	const Tour& parentTour;
	StopType stopType;
	bool primaryActivity;
	double arrivalTime;
	double departureTime;
	int stopMode;
	long stopLocation;
	bool inFirstHalfTour;
};

/**
 * A tour is a sequence of trips and activities of a person for a day.
 * A tour is assumed to start and end at the home location of the person.
 *
 */
class Tour {
public:
	Tour(StopType tourType)
	: tourType(tourType), usualLocation(false), subTour(false), parentTour(nullptr), tourMode(0), primaryActivityLocation(0), startTime(0), endTime(0)
	{}

	double getEndTime() const {
		return endTime;
	}

	void setEndTime(double endTime) {
		this->endTime = endTime;
	}

	const Tour& getParentTour() const {
		return *parentTour;
	}

	long getPrimaryActivityLocation() const {
		return primaryActivityLocation;
	}

	void setPrimaryActivityLocation(long primaryActivityLocation) {
		this->primaryActivityLocation = primaryActivityLocation;
	}

	double getStartTime() const {
		return startTime;
	}

	void setStartTime(double startTime) {
		this->startTime = startTime;
	}

	const std::deque<Stop*>& getStops() const {
		return stops;
	}

	void setStops(const std::deque<Stop*>& stops) {
		this->stops = stops;
	}

	bool isSubTour() const {
		return subTour;
	}

	void setSubTour(bool subTour) {
		this->subTour = subTour;
	}

	int getTourMode() const {
		return tourMode;
	}

	void setTourMode(int tourMode) {
		this->tourMode = tourMode;
	}

	StopType getTourType() const {
		return tourType;
	}

	void setTourType(StopType tourType) {
		this->tourType = tourType;
	}

	bool isUsualLocation() const {
		return usualLocation;
	}

	void setUsualLocation(bool usualLocation) {
		this->usualLocation = usualLocation;
	}

	void addStop(Stop* stop) {
		if(stop->isInFirstHalfTour()) {
			stops.push_front(stop);
		}
		else {
			stops.push_back(stop);
		}
	}

	void removeStop(Stop* stop) {
		if(stop->isInFirstHalfTour()) {
			stops.pop_front();
		}
		else {
			stops.pop_back();
		}
	}

private:
	StopType tourType;
	bool usualLocation;
	bool subTour;
	Tour* parentTour;
	int tourMode;
	long primaryActivityLocation;
	double startTime;
	double endTime;
	std::deque<Stop*> stops;
};


} // end namespace medium
} // end namespace sim_mob

