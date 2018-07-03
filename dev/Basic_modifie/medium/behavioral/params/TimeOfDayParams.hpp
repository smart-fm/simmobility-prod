//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <vector>

namespace sim_mob
{
namespace medium
{

/**
 * Simple class to store information pertaining tour time of day models
 * NOTE: This class is used by the mid-term behavior models.
 *
 * \author Harish Loganathan
 */
class TourTimeOfDayParams
{
public:
	TourTimeOfDayParams() :
			costHT1_AM(0), costHT1_PM(0), costHT1_OP(0), costHT2_AM(0), costHT2_PM(0), costHT2_OP(0), cbdOrgZone(false), cbdDestZone(false), tourMode(0)
	{
	}

	virtual ~TourTimeOfDayParams()
	{
	}

	/**
	 * Function to get a specific element from first half tour vector travelTimesFirstHalfTour
	 *
	 * @param index time window of the day
	 *
	 * @return travel time of the time window for first half tour
	 */
	double getTT_FirstHalfTour(int index) const
	{
		if (index < 1 || index > travelTimesFirstHalfTour.size())
		{
			throw std::runtime_error("TourTimeOfDayParams::getTT_FirstHalfTour() - Travel time requested for invalid time window");
		}
		return travelTimesFirstHalfTour[index - 1];
	}

	/**
	 * Function to get a specific element from second half tour vector travelTimesSecondHalfTour
	 *
	 * @param index time window of the day
	 *
	 * @return travel time of the time window for second half tour
	 */
	double getTT_SecondHalfTour(int index) const
	{
		if (index < 1 || index > travelTimesSecondHalfTour.size())
		{
			throw std::runtime_error("TourTimeOfDayParams::getTT_SecondHalfTour() - Travel time requested for invalid time window");
		}
		return travelTimesSecondHalfTour[index - 1];
	}

	double getCostHt1Am() const
	{
		return costHT1_AM;
	}

	void setCostHt1Am(double costHt1Am)
	{
		costHT1_AM = costHt1Am;
	}

	double getCostHt1Op() const
	{
		return costHT1_OP;
	}

	void setCostHt1Op(double costHt1Op)
	{
		costHT1_OP = costHt1Op;
	}

	double getCostHt1Pm() const
	{
		return costHT1_PM;
	}

	void setCostHt1Pm(double costHt1Pm)
	{
		costHT1_PM = costHt1Pm;
	}

	double getCostHt2Am() const
	{
		return costHT2_AM;
	}

	void setCostHt2Am(double costHt2Am)
	{
		costHT2_AM = costHt2Am;
	}

	double getCostHt2Op() const
	{
		return costHT2_OP;
	}

	void setCostHt2Op(double costHt2Op)
	{
		costHT2_OP = costHt2Op;
	}

	double getCostHt2Pm() const
	{
		return costHT2_PM;
	}

	void setCostHt2Pm(double costHt2Pm)
	{
		costHT2_PM = costHt2Pm;
	}

	int isCbdDestZone() const
	{
		return cbdDestZone;
	}

	void setCbdDestZone(int cbdDestZone)
	{
		this->cbdDestZone = cbdDestZone;
	}

	int isCbdOrgZone() const
	{
		return cbdOrgZone;
	}

	void setCbdOrgZone(int cbdOrgZone)
	{
		this->cbdOrgZone = cbdOrgZone;
	}

	int getTourMode() const
	{
		return tourMode;
	}

	void setTourMode(int tourMode)
	{
		this->tourMode = tourMode;
	}

	/**
	 * Vector storing the travel times for first and second half-tours in all half-hour windows within a day.
	 * The day starts at 0300Hrs and ends at 2659Hrs.
	 * The half-hour windows are 0300-0330, 0330-0400, 0400-0430, ... , 2600-2630, 2630-0300
	 * E.g.
	 * travelTimesFirstHalfTour[0] is the travel time for 0300hrs to 0330hrs (first half-hour window)
	 * travelTimesFirstHalfTour[47] is the travel time for 2630hrs to 0300hrs (last half-hour window)
	 *
	 */
	std::vector<double> travelTimesFirstHalfTour;
	std::vector<double> travelTimesSecondHalfTour;

private:
	double costHT1_AM;
	double costHT1_PM;
	double costHT1_OP;
	double costHT2_AM;
	double costHT2_PM;
	double costHT2_OP;
	int cbdOrgZone;
	int cbdDestZone;
	int tourMode;
};

/**
 * class to store parameters for the stop time of day model
 * NOTE: This class is used by the mid-term behavior models.
 *
 * \author Harish Loganathan
 */
class StopTimeOfDayParams
{
public:
	StopTimeOfDayParams(int stopType, bool firstBound) :
			stopType(stopType), firstBound(firstBound), numTimeWindows(48), todHigh(0.0), todLow(0.0), cbdOrgZone(false), cbdDestZone(false), stopMode(0)
	{
		for (unsigned i = 1; i <= numTimeWindows; i++)
		{
			availability.push_back(true);
		}
	}

	virtual ~StopTimeOfDayParams()
	{
	}

	double getTimeWindow(int choice_idx)
	{
		//There are 48 time windows from 3.25 (index 1) to 26.75 (index 48).
		//We can get the time window by applying a simple calculation on the index value.
		return ((choice_idx * 0.5) + 2.75);
	}

	int getFirstBound() const
	{
		return firstBound;
	}

	int getSecondBound() const
	{
		return !firstBound;
	}

	int getStopType() const
	{
		return stopType;
	}

	double getTodHigh() const
	{
		return todHigh;
	}

	void setTodHigh(double todHigh)
	{
		this->todHigh = todHigh;
	}

	double getTodLow() const
	{
		return todLow;
	}

	void setTodLow(double todLow)
	{
		this->todLow = todLow;
	}

	/**
	 * Function to get a specific element from vector travelTimes
	 *
	 * @param index of element
	 *
	 * @return the value from the vector if index < 48; returns -1 otherwise;
	 *
	 * NOTE: -1 is an invalid value for travel time and the caller must check for this value.
	 */
	double getTravelTime(unsigned index)
	{
		if (index <= numTimeWindows && index > 0)
		{
			return travelTimes[index - 1];
		}
		return -1;
	}

	/**
	 * Function to get a specific element from vector travelCost
	 *
	 * @param index of element
	 *
	 * @return the value from the vector if index < 48; returns -1 otherwise;
	 *
	 * NOTE: -1 is an invalid value for travel time and the caller must check for this value.
	 */
	double getTravelCost(unsigned index)
	{
		if (index <= numTimeWindows && index > 0)
		{
			return travelCost[index - 1];
		}
		return -1;
	}

	/**
	 * Sets the availabilities of all time windows before low tod and after high tod to false
	 */
	void updateAvailabilities()
	{
		size_t wndw = 0;
		for (size_t i = 0; i < numTimeWindows; i++)
		{
			wndw = i + 1;
			if (wndw < todLow || wndw > todHigh)
			{
				availability[i] = false;
			}
		}
	}

	/**
	 * Function to get the availability of an alternative
	 */
	int getAvailability(unsigned index)
	{
		if (index <= numTimeWindows && index > 0)
		{
			return availability[index - 1];
		}
		return 0; // anything else is unavailable
	}

	int isCbdDestZone() const
	{
		return cbdDestZone;
	}

	void setCbdDestZone(int cbdDestZone)
	{
		this->cbdDestZone = cbdDestZone;
	}

	int isCbdOrgZone() const
	{
		return cbdOrgZone;
	}

	void setCbdOrgZone(int cbdOrgZone)
	{
		this->cbdOrgZone = cbdOrgZone;
	}

	int getStopMode() const
	{
		return stopMode;
	}

	void setStopMode(int stopMode)
	{
		this->stopMode = stopMode;
	}

	/**
	 * Vector storing travel times for each half-hour time window from 3.25 to 26.75
	 *
	 * E.g.
	 * travelTimes[0] is the travel time for 3.25 window
	 * travelTimes[3] is the travel time for 4.75 window
	 */
	std::vector<double> travelTimes;

	/**
	 * Vector storing travel costs for each half-hour time window from 3.25 to 26.75
	 *
	 * E.g.
	 * travelCost[47] is the travel cost for 26.75 window
	 */
	std::vector<double> travelCost;

	/**
	 * availability vector
	 */
	std::vector<bool> availability;

private:
	int stopType;
	bool firstBound;
	double todHigh; // upper limit for time of day for this stop
	double todLow; // lower limit for time of day for this stop
	unsigned numTimeWindows;
	int cbdOrgZone;
	int cbdDestZone;
	int stopMode;
};
} // end namespace medium
} // end namespace sim_mob
