//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <deque>
#include <list>
#include <string>
#include <vector>
#include "behavioral/StopType.hpp"
#include "behavioral/StopType.hpp"
#include "util/LangHelpers.hpp"

namespace sim_mob
{
namespace medium
{

class Tour;

/**
 * Representation of an Activity for the demand simulator.
 *
 * \author Harish Loganathan
 */
class Stop
{
public:
	Stop(StopType stopType, const Tour& parentTour, bool primaryActivity, bool firstHalfTour) :
			stopType(stopType), parentTour(parentTour), primaryActivity(primaryActivity), arrivalTime(0), departureTime(0), stopMode(0), stopLocation(0),
				stopId(0), inFirstHalfTour(firstHalfTour)
	{
	}

	double getArrivalTime() const
	{
		return arrivalTime;
	}

	void setArrivalTime(double arrivalTime)
	{
		this->arrivalTime = arrivalTime;
	}

	double getDepartureTime() const
	{
		return departureTime;
	}

	void setDepartureTime(double departureTime)
	{
		this->departureTime = departureTime;
	}

	const Tour& getParentTour() const
	{
		return parentTour;
	}

	bool isPrimaryActivity() const
	{
		return primaryActivity;
	}

	void setPrimaryActivity(bool primaryActivity)
	{
		this->primaryActivity = primaryActivity;
	}

	int getStopLocation() const
	{
		return stopLocation;
	}

	void setStopLocation(int stopLocation)
	{
		this->stopLocation = stopLocation;
	}

	int getStopMode() const
	{
		return stopMode;
	}

	void setStopMode(int stopMode)
	{
		this->stopMode = stopMode;
	}

	StopType getStopType() const
	{
		return stopType;
	}

	/**
	 * returns the stop type as an integer
	 * used for passing stop type to lua models
	 */
	int getStopTypeID() const
	{
		switch (stopType)
		{
		case WORK:
			return 1;
		case EDUCATION:
			return 2;
		case SHOP:
			return 3;
		case OTHER:
			return 4;
		}
	}

	/**
	 * Returns the stop type in string format.
	 * used for writing output.
	 */
	std::string getStopTypeStr() const
	{
		switch (stopType)
		{
		case WORK:
			return "Work";
		case EDUCATION:
			return "Education";
		case SHOP:
			return "Shop";
		case OTHER:
			return "Other";
		}
	}

	void setStopType(StopType stopType)
	{
		this->stopType = stopType;
	}

	/**
	 * Sets the arrival and departure time of the stop
	 *
	 * @param arrivalTime arrival time (1 to 48)
	 * @param departureTime departure time (1 to 40)
	 */
	void allotTime(double arrivalTime, double departureTime)
	{
		this->arrivalTime = arrivalTime;
		this->departureTime = departureTime;
	}

	bool isInFirstHalfTour() const
	{
		return inFirstHalfTour;
	}

	int getStopLocationId() const
	{
		return stopId;
	}

	void setStopLocationId(int stopId)
	{
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
class Tour
{
public:
	Tour(StopType tourType, bool subTour = false) :
			tourType(tourType), usualLocation(false), subTour(subTour), tourMode(0), tourDestination(0), primaryStop(nullptr), startTime(0), endTime(0),
				firstTour(false)
	{
	}

	virtual ~Tour()
	{
		for (std::list<Stop*>::iterator i = stops.begin(); i != stops.end(); i++)
		{
			safe_delete_item(*i);
		}
		stops.clear();
	}

	double getEndTime() const
	{
		return endTime;
	}

	void setEndTime(double endTime)
	{
		this->endTime = endTime;
	}

	double getStartTime() const
	{
		return startTime;
	}

	void setStartTime(double startTime)
	{
		this->startTime = startTime;
	}

	bool isSubTour() const
	{
		return subTour;
	}

	void setSubTour(bool subTour)
	{
		this->subTour = subTour;
	}

	int getTourMode() const
	{
		return tourMode;
	}

	void setTourMode(int tourMode)
	{
		this->tourMode = tourMode;
	}

	StopType getTourType() const
	{
		return tourType;
	}

	/**
	 * Returns the tour type in string format.
	 * used for writing outputs.
	 */
	std::string getTourTypeStr() const
	{
		switch (tourType)
		{
		case WORK:
			return "Work";
		case EDUCATION:
			return "Education";
		case SHOP:
			return "Shop";
		case OTHER:
			return "Other";
		default:
			return "NULL";
		}
	}

	void setTourType(StopType tourType)
	{
		this->tourType = tourType;
	}

	bool isUsualLocation() const
	{
		return usualLocation;
	}

	void setUsualLocation(bool usualLocation)
	{
		this->usualLocation = usualLocation;
	}

	/**
	 * Adds a stop to the stops list appropriately depending on whether the stop
	 * is in the first half tour or the second.
	 *
	 * @param stop stop to be added
	 */
	void addStop(Stop* stop)
	{
		if (stop->isInFirstHalfTour())
		{
			stops.push_front(stop);
		}
		else
		{
			stops.push_back(stop);
		}
	}

	const Stop* getPrimaryStop() const
	{
		return primaryStop;
	}

	void setPrimaryStop(Stop* primaryStop)
	{
		this->primaryStop = primaryStop;
	}

	int getTourDestination() const
	{
		return tourDestination;
	}

	void setTourDestination(int tourDestination)
	{
		this->tourDestination = tourDestination;
	}

	bool isFirstTour() const
	{
		return firstTour;
	}

	void setFirstTour(bool firstTour)
	{
		this->firstTour = firstTour;
	}

	bool hasSubTours() const
	{
		return (!subTours.empty());
	}

	bool operator==(const Tour& rhs) const
	{
		return (this == &rhs);
	}

	bool operator!=(const Tour& rhs) const
	{
		return !(*this == rhs); //call == operator overload
	}

	/**
	 * List of stops in this tour.
	 * The relative ordering of stops in this list reflects the chronological order of the stops.
	 */
	std::list<Stop*> stops;

	/**
	 * List of sub tours for this tour.
	 * The relative ordering of sub-tours in this list reflects the chronological order of the tours.
	 */
	std::deque<Tour> subTours;

private:
	StopType tourType;
	bool usualLocation;
	bool subTour;
	int tourMode;
	int tourDestination;
	Stop* primaryStop;
	double startTime;
	double endTime;
	bool firstTour;
};

} // end namespace medium
} // end namespace sim_mob

