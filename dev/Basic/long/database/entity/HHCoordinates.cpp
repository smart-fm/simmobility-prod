/*
 * HHCoordinates.cpp
 *
 *  Created on: 11 Mar 2016
 *      Author: gishara
 */

#include "HHCoordinates.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

HHCoordinates::HHCoordinates(BigSerial houseHoldId, double centroidX, double centroidY): houseHoldId(houseHoldId), centroidX(centroidX),centroidY(centroidY){}

HHCoordinates::~HHCoordinates() {
}

double HHCoordinates::getCentroidX() const
{
	return centroidX;
}

void HHCoordinates::setCentroidX(double centroidX)
{
	this->centroidX = centroidX;
}

double HHCoordinates::getCentroidY() const
{
	return centroidY;
}

void HHCoordinates::setCentroidY(double centroidY)
{
	this->centroidY = centroidY;
}

BigSerial HHCoordinates::getHouseHoldId() const
{
	return houseHoldId;
}

void HHCoordinates::setHouseHoldId(BigSerial houseHoldId)
{
	this->houseHoldId = houseHoldId;
}



