//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ParkingSlot.hpp"

#include "RoadSegment.hpp"

using namespace sim_mob;

ParkingSlot::ParkingSlot() : RoadItem(), offset(0), length(0), isOccupied(false), parentSegment(nullptr)
{
	roadItemId = 0;
	roadSegmentId = 0;
}

ParkingSlot::~ParkingSlot()
{
}

double ParkingSlot::getOffset() const
{
	return this->offset;
}

void ParkingSlot::setOffset(double offset)
{
	this->offset = offset;
}

double ParkingSlot::getLength() const
{
	return this->length;
}

void ParkingSlot::setLength(double length)
{
	this->length = length;
}

bool ParkingSlot::isVacant() const
{
	return !(this->isOccupied);
}

void ParkingSlot::setIsOccupied(bool occupied)
{
	this->isOccupied = occupied;
}

const RoadSegment* ParkingSlot::getParentSegment() const
{
	return this->parentSegment;
}

void ParkingSlot::setParentSegment(const RoadSegment *rdSegment)
{
	this->parentSegment = rdSegment;
}