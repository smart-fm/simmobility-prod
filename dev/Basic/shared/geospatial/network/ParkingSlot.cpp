//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ParkingSlot.hpp"

#include "RoadSegment.hpp"

using namespace sim_mob;

ParkingSlot::ParkingSlot() : RoadItem(), accessSegmentId(0), egressSegmentId(0), offset(0), length(0), capacity(0), isOccupied(false), 
accessSegment(nullptr), egressSegment(nullptr)
{
	roadItemId = 0;
	roadSegmentId = 0;
}

ParkingSlot::~ParkingSlot()
{
}

const unsigned int ParkingSlot::getParkingSlotId() const
{
	return roadItemId;
}

const unsigned int ParkingSlot::getAccessSegmentId() const
{
	return accessSegmentId;
}

void ParkingSlot::setAccessSegmentId(unsigned int segmentId)
{
	accessSegmentId = roadSegmentId = segmentId;	
}

const unsigned int ParkingSlot::getEgressSegmentId() const
{
	return egressSegmentId;
}

void ParkingSlot::setEgressSegmentId(unsigned int segmentId)
{
	egressSegmentId = segmentId;
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

const unsigned int ParkingSlot::getCapacity() const
{
	return this->capacity;
}

void ParkingSlot::setCapacity(unsigned int capacityPCU)
{
	this->capacity = capacity;
}

bool ParkingSlot::isVacant() const
{
	return !(this->isOccupied);
}

void ParkingSlot::setIsOccupied(bool occupied)
{
	this->isOccupied = occupied;
}

const RoadSegment* ParkingSlot::getAccessSegment() const
{
	return this->accessSegment;
}

void ParkingSlot::setAccessSegment(const RoadSegment *rdSegment)
{
	this->accessSegment = rdSegment;
}

const RoadSegment* ParkingSlot::getEgressSegment() const
{
	return this->egressSegment;
}

void ParkingSlot::setEgressSegment(const RoadSegment *rdSegment)
{
	this->egressSegment = rdSegment;
}