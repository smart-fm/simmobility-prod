//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * StopGenerationParams.cpp
 *
 *  Created on: Nov 22, 2013
 *      Author: Harish Loganathan
 */

#include "StopGenerationParams.hpp"
using namespace sim_mob;
using namespace sim_mob::medium;

sim_mob::medium::StopGenerationParams::StopGenerationParams(PersonParams& personParams, Tour& tour, Stop& primaryActivity)
: personParams(personParams), tour(tour), primaryActivity(primaryActivity), firstTour(true), firstHalfTour(true), stopCounter(0),
  workStopAvailability(0), eduStopAvailability(0), shopStopAvailability(0), otherStopAvailability(0),
  numRemainingTours(-1), distance(-1.0)	/*distance, initialized with invalid values*/
{}

sim_mob::medium::StopGenerationParams::~StopGenerationParams()
{}

int sim_mob::medium::StopGenerationParams::getTourType() {
	switch(tour.getTourType()) {
		case WORK: return 1;
		case EDUCATION: return 2;
		case SHOP: return 3;
		case OTHER: return 4;
	}
}

int sim_mob::medium::StopGenerationParams::isDriver() {
	return (tour.getTourMode() == 4);
}

int sim_mob::medium::StopGenerationParams::isPassenger() {
	return (tour.getTourMode() == 5 || tour.getTourMode() == 6);
}

int sim_mob::medium::StopGenerationParams::isPublicTransitCommuter() {
	return (tour.getTourMode() >= 1 && tour.getTourMode() <= 3);
}

int sim_mob::medium::StopGenerationParams::isFirstTour() {
	return firstTour;
}

void sim_mob::medium::StopGenerationParams::setFirstTour(bool firstTour) {
	this->firstTour = firstTour;
}

int sim_mob::medium::StopGenerationParams::getNumRemainingTours() {
	return numRemainingTours;
}

void sim_mob::medium::StopGenerationParams::setNumRemainingTours(int numRemainingTours) {
	this->numRemainingTours = numRemainingTours;
}

double sim_mob::medium::StopGenerationParams::getDistance() {
	return distance;
}

void sim_mob::medium::StopGenerationParams::setDistance(double distance) {
	this->distance = distance;
}

int sim_mob::medium::StopGenerationParams::getP_700a_930a() {
	if(firstHalfTour) {
		return (primaryActivity.getArrivalTime() > 7 && primaryActivity.getArrivalTime() <= 9.5);
	}
	else {
		return (primaryActivity.getDepartureTime() > 7 && primaryActivity.getDepartureTime() <= 9.5);
	}
}

int sim_mob::medium::StopGenerationParams::getP_930a_1200a() {
	if(firstHalfTour) {
		return (primaryActivity.getArrivalTime() > 9.5 && primaryActivity.getArrivalTime() <= 12);
	}
	else { //secondHalfTour
		return (primaryActivity.getDepartureTime() > 9.5 && primaryActivity.getDepartureTime() <= 12);
	}
}

int sim_mob::medium::StopGenerationParams::getP_300p_530p() {
	if(firstHalfTour) {
		return (primaryActivity.getArrivalTime() > 15 && primaryActivity.getArrivalTime() <= 17.5);
	}
	else { //secondHalfTour
		return (primaryActivity.getDepartureTime() > 15 && primaryActivity.getDepartureTime() <= 17.5);
	}
}

int sim_mob::medium::StopGenerationParams::getP_530p_730p() {
	if(firstHalfTour) {
		return (primaryActivity.getArrivalTime() > 17.5 && primaryActivity.getArrivalTime() <= 19.5);
	}
	else { //secondHalfTour
		return (primaryActivity.getDepartureTime() > 17.5 && primaryActivity.getDepartureTime() <= 19.5);
	}
}

int sim_mob::medium::StopGenerationParams::getP_730p_1000p() {
	if(firstHalfTour) {
		return (primaryActivity.getArrivalTime() > 19.5 && primaryActivity.getArrivalTime() <= 22);
	}
	else { //secondHalfTour
		return (primaryActivity.getDepartureTime() > 19.5 && primaryActivity.getDepartureTime() <= 22);
	}
}

int sim_mob::medium::StopGenerationParams::getP_1000p_700a() {
	if(firstHalfTour) {
		return (
				(primaryActivity.getArrivalTime() > 22 && primaryActivity.getArrivalTime() <= 27) ||
				(primaryActivity.getArrivalTime() > 0 && primaryActivity.getArrivalTime() <= 7)
				);
	}
	else { //secondHalfTour
		return (
				(primaryActivity.getDepartureTime() > 22 && primaryActivity.getDepartureTime() <= 27) ||
				(primaryActivity.getDepartureTime() > 0 && primaryActivity.getDepartureTime() <= 7)
				);
	}
}

int sim_mob::medium::StopGenerationParams::getFirstBound() {
	return firstHalfTour;
}

int sim_mob::medium::StopGenerationParams::getSecondBound() {
	return !firstHalfTour;
}

void sim_mob::medium::StopGenerationParams::setFirstHalfTour(bool firstHalfTour) {
	this->firstHalfTour = firstHalfTour;
}

int sim_mob::medium::StopGenerationParams::getFirstStop() {
	return (stopCounter == 0);
}

int sim_mob::medium::StopGenerationParams::getSecondStop() {
	return (stopCounter == 1);
}

int sim_mob::medium::StopGenerationParams::getThreePlusStop() {
	return (stopCounter >= 2);
}

void sim_mob::medium::StopGenerationParams::setStopCounter(int stopCounter) {
	this->stopCounter = stopCounter;
}

int sim_mob::medium::StopGenerationParams::getWorkStopAvailability() {
	return workStopAvailability;
}

int sim_mob::medium::StopGenerationParams::getEduStopAvailability() {
	return eduStopAvailability;
}

int sim_mob::medium::StopGenerationParams::getShopStopAvailability() {
	return shopStopAvailability;
}

int sim_mob::medium::StopGenerationParams::getOtherStopAvailability() {
	return otherStopAvailability;
}

void sim_mob::medium::StopGenerationParams::setEduStopAvailability(int eduStopAvailability) {
	this->eduStopAvailability = eduStopAvailability;
}

void sim_mob::medium::StopGenerationParams::setOtherStopAvailability(int otherStopAvailability) {
	this->otherStopAvailability = otherStopAvailability;
}

void sim_mob::medium::StopGenerationParams::setShopStopAvailability(int shopStopAvailability) {
	this->shopStopAvailability = shopStopAvailability;
}

void sim_mob::medium::StopGenerationParams::setWorkStopAvailability(int workStopAvailability) {
	this->workStopAvailability = workStopAvailability;
}

