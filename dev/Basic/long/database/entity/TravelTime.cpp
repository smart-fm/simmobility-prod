/*
 * TravelTime.cpp
 *
 *  Created on: 11 Mar 2016
 *      Author: gishara
 */
#include "TravelTime.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

TravelTime::TravelTime(BigSerial origin, BigSerial destination, double carTravelTime,double publicTravelTime): origin(origin), destination(destination),carTravelTime(carTravelTime),publicTravelTime(publicTravelTime){}

TravelTime::~TravelTime() {}


double TravelTime::getCarTravelTime() const
{
	return carTravelTime;
}

void TravelTime::setCarTravelTime(double carTravelTime)
{
	this->carTravelTime = carTravelTime;
}

BigSerial TravelTime::getDestination() const
{
	return destination;
}

void TravelTime::setDestination(BigSerial destination)
{
	this->destination = destination;
}

BigSerial TravelTime::getOrigin() const
{
	return origin;
}

void TravelTime::setOrigin(BigSerial origin)
{
	this->origin = origin;
}

double TravelTime::getPublicTravelTime() const
{
	return publicTravelTime;
}

void TravelTime::setPublicTravelTime(double publicTravelTime)
{
	this->publicTravelTime = publicTravelTime;
}




