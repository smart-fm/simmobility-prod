//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PredayClasses.hpp
 *
 *  Created on: Nov 11, 2013
 *      Author: Harish Loganathan
 */

#include <deque>
#include <string>

#include <boost/unordered_map.hpp>

namespace sim_mob {
namespace medium {

enum TourType {
	WORK_TOUR, EDUCATION_TOUR, SHOP_TOUR, OTHER_TOUR
};

enum StopType {
	WORK_STOP, EDUCATION_STOP, SHOP_STOP, OTHER_STOP
};

class Tour;

class Stop {
public:
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

	const std::string& getStopMode() const {
		return stopMode;
	}

	void setStopMode(const std::string& stopMode) {
		this->stopMode = stopMode;
	}

	StopType getStopType() const {
		return stopType;
	}

	void setStopType(StopType stopType) {
		this->stopType = stopType;
	}

private:
	const Tour& parentTour;
	StopType stopType;
	bool primaryActivity;
	double arrivalTime;
	double departureTime;
	std::string stopMode;
	long stopLocation;
};

class Tour {
public:
	double getEndTime() const {
		return endTime;
	}

	void setEndTime(double endTime) {
		this->endTime = endTime;
	}

	const Tour& getParentTour() const {
		return parentTour;
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

	const std::string& getTourMode() const {
		return tourMode;
	}

	void setTourMode(const std::string& tourMode) {
		this->tourMode = tourMode;
	}

	TourType getTourType() const {
		return tourType;
	}

	void setTourType(TourType tourType) {
		this->tourType = tourType;
	}

	bool isUsualLocation() const {
		return usualLocation;
	}

	void setUsualLocation(bool usualLocation) {
		this->usualLocation = usualLocation;
	}

private:
	TourType tourType;
	bool usualLocation;
	bool subTour;
	Tour& parentTour;
	std::string tourMode;
	long primaryActivityLocation;
	double startTime;
	double endTime;
	std::deque<Stop*> stops;
};
} // end namespace medium
} // end namespace sim_mob

