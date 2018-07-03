/*
 * ParcelAmenities.cpp
 *
 *  Created on: Dec 12, 2014
 *      Author: gishara
 */
#include "ParcelAmenities.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

ParcelAmenities::ParcelAmenities(): fmParcelId(INVALID_ID), nearestMRT(EMPTY_STR), distanceToMRT(0), distanceToBus(0),
		distanceToExpress(0),distanceToPMS30(0), distanceToCBD(0), distanceToMall(0), distanceToJob(0),
		mrt_200m(false), mrt_400m(false), express_200m(false),bus_200m(false), bus_400m(false), pms_1km(false) {}

ParcelAmenities::~ParcelAmenities() {
}

int ParcelAmenities::hasPms_1km() const {
    return pms_1km;
}

int ParcelAmenities::hasBus_400m() const {
    return bus_400m;
}

int ParcelAmenities::hasBus_200m() const {
    return bus_200m;
}

int ParcelAmenities::hasExpress_200m() const {
    return express_200m;
}

int ParcelAmenities::hasMRT_400m() const {
    return mrt_400m;
}

int ParcelAmenities::hasMRT_200m() const {
    return mrt_200m;
}

double ParcelAmenities::getDistanceToJob() const {
    return distanceToJob;
}

double ParcelAmenities::getDistanceToMall() const {
    return distanceToMall;
}

double ParcelAmenities::getDistanceToCBD() const {
    return distanceToCBD;
}

double ParcelAmenities::getDistanceToPMS30() const {
    return distanceToPMS30;
}

double ParcelAmenities::getDistanceToExpress() const {
    return distanceToExpress;
}

double ParcelAmenities::getDistanceToBus() const {
    return distanceToBus;
}

double ParcelAmenities::getDistanceToMRT() const {
    return distanceToMRT;
}

const std::string& ParcelAmenities::getNearestMRT() const {
	return nearestMRT;
}

BigSerial ParcelAmenities::getFmParcelId() const {
	return fmParcelId;
}



