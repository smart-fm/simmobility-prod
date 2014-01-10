//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * StopGenerationParams.hpp
 *
 *  Created on: Nov 22, 2013
 *      Author: Harish Loganathan
 */

#pragma once
#include "behavioral/PredayClasses.hpp"

namespace sim_mob {
namespace medium {

/**
 * Simple class to store information pertaining intermediate stop generation model
 * \note This class is used by the mid-term behavior models.
 *
 * \author Harish Loganathan
 */
class StopGenerationParams {
public:
	StopGenerationParams(Tour& tour, Stop* primaryActivity, const boost::unordered_map<std::string, bool>& dayPattern)
	: tourMode(tourMode), primActivityArrivalTime(primaryActivity->getArrivalTime()), primActivityDeptTime(primaryActivity->getDepartureTime()),
	  firstTour(true), firstHalfTour(true), stopCounter(0), hasSubtour(0),
	  numRemainingTours(-1), distance(-1.0)	/*distance, initialized with invalid values*/
	{
		switch (tour.getTourType()) {
		case WORK:
			tourType = 1;
			break;
		case EDUCATION:
			tourType = 2;
			break;
		case SHOP:
			tourType = 3;
			break;
		case OTHER:
			tourType = 4;
			break;
		}

		workStopAvailability = dayPattern.at("WorkI");
		eduStopAvailability = dayPattern.at("EduI");
		shopStopAvailability = dayPattern.at("ShopI");
		otherStopAvailability = dayPattern.at("OthersI");
	}

	virtual ~StopGenerationParams() {}

	int getTourType() const {
		return tourType;
	}

	int isDriver() const {
		return (tourMode == 4);
	}

	int isPassenger() const {
		return (tourMode == 5 || tourMode == 6);
	}

	int isPublicTransitCommuter() const {
		return (tourMode >= 1 && tourMode <= 3);
	}

	int isFirstTour() const {
		return firstTour;
	}

	void setFirstTour(bool firstTour) {
		this->firstTour = firstTour;
	}

	int getNumRemainingTours() const {
		return numRemainingTours;
	}

	void setNumRemainingTours(int numRemainingTours) {
		this->numRemainingTours = numRemainingTours;
	}

	double getDistance() const {
		return distance;
	}

	void setDistance(double distance) {
		this->distance = distance;
	}

	int getP_700a_930a() const {
		if(firstHalfTour) {
			return (primActivityArrivalTime > 7 && primActivityArrivalTime <= 9.5);
		}
		else {
			return (primActivityDeptTime > 7 && primActivityDeptTime <= 9.5);
		}
	}

	int getP_930a_1200a() const {
		if(firstHalfTour) {
			return (primActivityArrivalTime > 9.5 && primActivityArrivalTime <= 12);
		}
		else { //secondHalfTour
			return (primActivityDeptTime > 9.5 && primActivityDeptTime <= 12);
		}
	}

	int getP_300p_530p() const {
		if(firstHalfTour) {
			return (primActivityArrivalTime > 15 && primActivityArrivalTime <= 17.5);
		}
		else { //secondHalfTour
			return (primActivityDeptTime > 15 && primActivityDeptTime <= 17.5);
		}
	}

	int getP_530p_730p() const {
		if(firstHalfTour) {
			return (primActivityArrivalTime > 17.5 && primActivityArrivalTime <= 19.5);
		}
		else { //secondHalfTour
			return (primActivityDeptTime > 17.5 && primActivityDeptTime <= 19.5);
		}
	}

	int getP_730p_1000p() const {
		if(firstHalfTour) {
			return (primActivityArrivalTime > 19.5 && primActivityArrivalTime <= 22);
		}
		else { //secondHalfTour
			return (primActivityDeptTime > 19.5 && primActivityDeptTime <= 22);
		}
	}

	int getP_1000p_700a() const {
		if(firstHalfTour) {
			return (
					(primActivityArrivalTime > 22 && primActivityArrivalTime <= 27) ||
					(primActivityArrivalTime > 0 && primActivityArrivalTime <= 7)
					);
		}
		else { //secondHalfTour
			return (
					(primActivityDeptTime > 22 && primActivityDeptTime <= 27) ||
					(primActivityDeptTime > 0 && primActivityDeptTime <= 7)
					);
		}
	}

	int getFirstBound() const {
		return firstHalfTour;
	}

	int getSecondBound() const {
		return !firstHalfTour;
	}

	void setFirstHalfTour(bool firstHalfTour) {
		this->firstHalfTour = firstHalfTour;
	}

	int getFirstStop() const {
		return (stopCounter == 0);
	}

	int getSecondStop() const {
		return (stopCounter == 1);
	}

	int getThreePlusStop() const {
		return (stopCounter >= 2);
	}

	void setStopCounter(int stopCounter) {
		this->stopCounter = stopCounter;
	}

	int isAvailable(int stopType) const {
		switch(stopType) {
		case 1:
			return workStopAvailability;
		case 2:
			return eduStopAvailability;
		case 3:
			return shopStopAvailability;
		case 4:
			return otherStopAvailability;
		case 5:
			return 1; //Quit alternative is always available
		}
	}

	void setEduStopAvailability(int eduStopAvailability) {
		this->eduStopAvailability = eduStopAvailability;
	}

	void setOtherStopAvailability(int otherStopAvailability) {
		this->otherStopAvailability = otherStopAvailability;
	}

	void setShopStopAvailability(int shopStopAvailability) {
		this->shopStopAvailability = shopStopAvailability;
	}

	void setWorkStopAvailability(int workStopAvailability) {
		this->workStopAvailability = workStopAvailability;
	}

	int getHasSubtour() const {
		return hasSubtour;
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
	int stopCounter;
	int hasSubtour;

	int workStopAvailability;
	int eduStopAvailability;
	int shopStopAvailability;
	int otherStopAvailability;
};
} //end namespace medium
} // end namespace sim_mob
