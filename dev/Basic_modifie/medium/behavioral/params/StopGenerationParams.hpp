//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <stdint.h>
#include "behavioral/PredayClasses.hpp"
#include "conf/ConfigManager.hpp"

namespace sim_mob
{
namespace medium
{

/**
 * Simple class to store information pertaining to intermediate stop generation model
 * NOTE: This class is used by the mid-term behavior models.
 *
 * \author Harish Loganathan
 */
class StopGenerationParams
{
public:
        StopGenerationParams(const Tour& tour, const Stop* primaryActivity, const std::unordered_map<int, bool>& dayPatternStops) :
			tourMode(tour.getTourMode()), primActivityArrivalTime(primaryActivity->getArrivalTime()), primActivityDeptTime(primaryActivity->getDepartureTime()),
                                firstTour(tour.isFirstTour()), firstHalfTour(true), numPreviousStops(0), hasSubtour(tour.hasSubTours()), dayPatternStops(dayPatternStops), tourType(tour.getTourType()),
                                numRemainingTours(-1), distance(-1.0), timeWindowFirstBound(-1), timeWindowSecondBound(-1) /*initialized with invalid values*/
	{
	}

	virtual ~StopGenerationParams()
	{
	}

	int getTourType() const
	{
		return tourType;
	}

	int isDriver() const
	{
                return (ConfigManager::GetInstance().FullConfig().getTravelModeConfig(tourMode).type == PVT_CAR_MODE);
	}

	int isPassenger() const
	{
                return (ConfigManager::GetInstance().FullConfig().getTravelModeConfig(tourMode).type == SHARING_MODE);
	}

	int isPublicTransitCommuter() const
	{
            return (ConfigManager::GetInstance().FullConfig().getTravelModeConfig(tourMode).type == PT_TRAVEL_MODE);
	}

	int isFirstTour() const
	{
		return firstTour;
	}

	void setFirstTour(bool firstTour)
	{
		this->firstTour = firstTour;
	}

	int getNumRemainingTours() const
	{
		return numRemainingTours;
	}

	void setNumRemainingTours(int numRemainingTours)
	{
		this->numRemainingTours = numRemainingTours;
	}

	double getDistance() const
	{
		return distance;
	}

	void setDistance(double distance)
	{
		this->distance = distance;
	}

	int getP_700a_930a() const
	{
		if (firstHalfTour)
		{
			return (primActivityArrivalTime > 7 && primActivityArrivalTime <= 9.5);
		}
		else
		{
			return (primActivityDeptTime > 7 && primActivityDeptTime <= 9.5);
		}
	}

	int getP_930a_1200a() const
	{
		if (firstHalfTour)
		{
			return (primActivityArrivalTime > 9.5 && primActivityArrivalTime <= 12);
		}
		else
		{ //secondHalfTour
			return (primActivityDeptTime > 9.5 && primActivityDeptTime <= 12);
		}
	}

	int getP_300p_530p() const
	{
		if (firstHalfTour)
		{
			return (primActivityArrivalTime > 15 && primActivityArrivalTime <= 17.5);
		}
		else
		{ //secondHalfTour
			return (primActivityDeptTime > 15 && primActivityDeptTime <= 17.5);
		}
	}

	int getP_530p_730p() const
	{
		if (firstHalfTour)
		{
			return (primActivityArrivalTime > 17.5 && primActivityArrivalTime <= 19.5);
		}
		else
		{ //secondHalfTour
			return (primActivityDeptTime > 17.5 && primActivityDeptTime <= 19.5);
		}
	}

	int getP_730p_1000p() const
	{
		if (firstHalfTour)
		{
			return (primActivityArrivalTime > 19.5 && primActivityArrivalTime <= 22);
		}
		else
		{ //secondHalfTour
			return (primActivityDeptTime > 19.5 && primActivityDeptTime <= 22);
		}
	}

	int getP_1000p_700a() const
	{
		if (firstHalfTour)
		{
			return ((primActivityArrivalTime > 22 && primActivityArrivalTime <= 27) || (primActivityArrivalTime > 0 && primActivityArrivalTime <= 7));
		}
		else
		{ //secondHalfTour
			return ((primActivityDeptTime > 22 && primActivityDeptTime <= 27) || (primActivityDeptTime > 0 && primActivityDeptTime <= 7));
		}
	}

	int getFirstBound() const
	{
		return firstHalfTour;
	}

	int getSecondBound() const
	{
		return !firstHalfTour;
	}

	void setFirstHalfTour(bool firstHalfTour)
	{
		this->firstHalfTour = firstHalfTour;
	}

	int getFirstStop() const
	{
		return (numPreviousStops == 0);
	}

	int getSecondStop() const
	{
		return (numPreviousStops == 1);
	}

	int getThreePlusStop() const
	{
		return (numPreviousStops >= 2);
	}

	void setNumPreviousStops(uint8_t numPreviousStops)
	{
		this->numPreviousStops = numPreviousStops;
	}

	int isAvailable(int stopType) const
	{
                if (dayPatternStops.find(stopType) == dayPatternStops.end())
                {
                    return 1;
                }

                return dayPatternStops.at(stopType);
	}

	int getHasSubtour() const
	{
		return hasSubtour;
	}

	double getTimeWindowFirstBound() const
	{
		return timeWindowFirstBound;
	}

	void setTimeWindowFirstBound(double timeWindowFirstBound)
	{
		this->timeWindowFirstBound = timeWindowFirstBound;
	}

	double getTimeWindowSecondBound() const
	{
		return timeWindowSecondBound;
	}

	void setTimeWindowSecondBound(double timeWindowSecondBound)
	{
		this->timeWindowSecondBound = timeWindowSecondBound;
	}

private:

	int tourType;
	int tourMode;
	double primActivityArrivalTime;
	double primActivityDeptTime;
	bool firstTour;
	int numRemainingTours;
	double distance;
	bool firstHalfTour;
	uint8_t numPreviousStops;
	bool hasSubtour;
	double timeWindowFirstBound;
	double timeWindowSecondBound;

        const std::unordered_map<int, bool>& dayPatternStops;
};
} //end namespace medium
} // end namespace sim_mob
