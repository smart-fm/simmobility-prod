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
#include <vector>
#include "behavioral/PredayClasses.hpp"
#include "PersonParams.hpp"

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
	StopGenerationParams(PersonParams& personParams, Tour& tour, Stop& primaryActivity);
	virtual ~StopGenerationParams();

	int getTourType();
	int isDriver();
	int isPassenger();
	int isPublicTransitCommuter();
	int isFirstTour();
	void setFirstTour(bool firstTour);
	int getNumRemainingTours();
	void setNumRemainingTours(int numRemainingTours);
	double getDistance();
	void setDistance(double distance);
	int getP_700a_930a();
	int getP_930a_1200a();
	int getP_300p_530p();
	int getP_530p_730p();
	int getP_730p_1000p();
	int getP_1000p_700a();
	int getFirstBound();
	int getSecondBound();
	void setFirstHalfTour(bool firstHalfTour);
	int getFirstStop();
	int getSecondStop();
	int getThreePlusStop();
	void setStopCounter(int stopCounter);
	int getWorkStopAvailability();
	int getEduStopAvailability();
	int getShopStopAvailability();
	int getOtherStopAvailability();
	void setEduStopAvailability(int eduStopAvailability);
	void setOtherStopAvailability(int otherStopAvailability);
	void setShopStopAvailability(int shopStopAvailability);
	void setWorkStopAvailability(int workStopAvailability);

private:
	PersonParams& personParams;
	Tour& tour;
	Stop& primaryActivity;
	bool firstTour;
	int numRemainingTours;
	double distance;
	bool firstHalfTour;
	int stopCounter;

	int workStopAvailability;
	int eduStopAvailability;
	int shopStopAvailability;
	int otherStopAvailability;
};
} //end namespace medium
} // end namespace sim_mob
