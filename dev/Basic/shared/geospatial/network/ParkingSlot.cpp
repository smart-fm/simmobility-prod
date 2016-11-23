//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ParkingSlot.hpp"

#include "RoadSegment.hpp"

using namespace sim_mob;

ParkingSlot::ParkingSlot(unsigned int id, unsigned int roadSegmentId) : RoadItem(id, 0, 0 , roadSegmentId)
{
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