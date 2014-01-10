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
 * startTime and endTime are integral numbers from 1 to 48
 * The times are considered in half hour windows.
 * There are 48 half hour windows in a day.
 *
 * x.25 represents a time value between x:00 - x:29
 * x.75 represents a time value between x:30 and x:59
 * - where x varies from 3 to 26
 *
 * The startTime must be lesser than or equal to endTime.
 *
 * 3.25 (0300 to 0329 hrs) is time 1
 * 3.75 (0330 to 0359 hrs) is time 2
 * 4.25 (0400 to 0429 hrs) is time 3
 * ... so on
 * 23.75 (1130 to 1159 hrs) is time 42
 * 24.25 (1200 to 1229 hrs) is time 43
 * ... so on
 * 26.75 (0230 to 0259 hrs) is time 48
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

	/**
	 * This vector is used as lookup for obtaining the start and end time of the time window chosen from the time of day model
	 * There are 48 half-hour windows in a day. Each half hour window can be a start time of a time-window and any half-hour window
	 * after the start time in the same can be an end time of a time-window. Therefore there are (48 * (48+1) / 2) = 1176 time windows in a day.
	 * This vector has 1176 elements.
	 */
	static const std::vector<TimeWindowAvailability> timeWindowsLookup;

private:
	double startTime;
	double endTime;
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
	: stopType(stopType), parentTour(parentTour), primaryActivity(primaryActivity), arrivalTime(0), departureTime(0), stopMode(0),
	  stopLocation(0), stopId(0), inFirstHalfTour(firstHalfTour)
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

	int getStopLocation() const {
		return stopLocation;
	}

	void setStopLocation(int stopLocation) {
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

	/**
	 * returns the stop type as an integer
	 * used for passing stop type to lua models
	 */
	int getStopTypeID() const {
		switch(stopType) {
		case WORK: return 1;
		case EDUCATION: return 2;
		case SHOP: return 3;
		case OTHER: return 4;
		}
	}

	/**
	 * Returns the stop type in string format.
	 * used for writing output.
	 */
	std::string getStopTypeStr() const {
		switch(stopType) {
		case WORK: return "Work";
		case EDUCATION: return "Education";
		case SHOP: return "Shop";
		case OTHER: return "Other";
		}
	}

	void setStopType(StopType stopType) {
		this->stopType = stopType;
	}

	/**
	 * Sets the arrival and departure time of the stop
	 *
	 * @param arrivalTime arrival time
	 * @param departureTime departure time
	 */
	void allotTime(double arrivalTime, double departureTime) {
		this->arrivalTime = arrivalTime;
		this->departureTime = departureTime;
	}

	bool isInFirstHalfTour() const {
		return inFirstHalfTour;
	}

	int getStopLocationId() const {
		return stopId;
	}

	void setStopLocationId(int stopId) {
		this->stopId = stopId;
	}

private:
	const Tour& parentTour;
	StopType stopType;
	bool primaryActivity;
	double arrivalTime;
	double departureTime;
	int stopMode;
	int stopLocation;
	int stopId;
	bool inFirstHalfTour;
};

/**
 * A tour is a sequence of trips and activities of a person for a day.
 * A tour is assumed to start and end at the home location of the person.
 *
 *\author Harish Loganathan
 */
class Tour {
public:
	Tour(StopType tourType)
	: tourType(tourType), usualLocation(false), subTour(false), parentTour(nullptr), tourMode(0), tourDestination(0), primaryStop(nullptr), startTime(0), endTime(0)
	{}

	virtual ~Tour() {
		for(std::deque<Stop*>::iterator i = stops.begin(); i!=stops.end(); i++) {
			safe_delete_item(*i);
		}
		stops.clear();
	}

	double getEndTime() const {
		return endTime;
	}

	void setEndTime(double endTime) {
		this->endTime = endTime;
	}

	const Tour& getParentTour() const {
		return *parentTour;
	}

	double getStartTime() const {
		return startTime;
	}

	void setStartTime(double startTime) {
		this->startTime = startTime;
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

	/**
	 * Returns the tour type in string format.
	 * used for writing outputs.
	 */
	std::string getTourTypeStr() const {
		switch(tourType) {
		case WORK: return "Work";
		case EDUCATION: return "Education";
		case SHOP: return "Shop";
		case OTHER: return "Other";
		}
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

	/**
	 * Adds a stop to the stops list appropriately depending on whether the stop
	 * is in the first half tour or the second.
	 *
	 * @param stop stop to be added
	 */
	void addStop(Stop* stop) {
		if(stop->isInFirstHalfTour()) {
			stops.push_front(stop);
		}
		else {
			stops.push_back(stop);
		}
	}

	/**
	 * removes a stop to the stops list appropriately depending on whether the stop
	 * is in the first half tour or the second.
	 *
	 * @param stop stop to be removed
	 */
	void removeStop(Stop* stop) {
		if(stop->isInFirstHalfTour()) {
			stops.pop_front();
		}
		else {
			stops.pop_back();
		}
	}

	const Stop* getPrimaryStop() const {
		return primaryStop;
	}

	void setPrimaryStop(Stop* primaryStop) {
		this->primaryStop = primaryStop;
	}

	int getTourDestination() const {
		return tourDestination;
	}

	void setTourDestination(int tourDestination) {
		this->tourDestination = tourDestination;
	}

	/**
	 * list of stops in this tour.
	 * relative ordering of stops according to the time of day in which they occur is maintained in this list.
	 */
	std::deque<Stop*> stops;

private:
	StopType tourType;
	bool usualLocation;
	bool subTour;
	Tour* parentTour; // in case of sub tours
	int tourMode;
	int tourDestination;
	Stop* primaryStop;
	double startTime;
	double endTime;
};


} // end namespace medium
} // end namespace sim_mob

