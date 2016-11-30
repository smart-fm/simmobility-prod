//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "RoadItem.hpp"

namespace sim_mob
{

class RoadSegment;

/**
 * Represents the road side parking space
 * \author Neeraj
 */
class ParkingSlot : public RoadItem
{
private:
	/**The id of the road segment from where the parking can be accessed*/
	unsigned int accessSegmentId;

	/**The id of the road segment at which the parked vehicle exits*/
	unsigned int egressSegmentId;

	/**The distance of the parking slot from the start of the segment*/
	double offset;

	/**The length of the parking slot in metre*/
	double length;

	/**The type of the parking space*/

	/**The capacity of the parking slot in PCU (passenger car units)*/
	unsigned int capacity;

	/**Indicates whether the parking slot is occupied*/
	bool isOccupied;

	/**The access road segment*/
	const RoadSegment *accessSegment;

	/**The egress road segment*/
	const RoadSegment *egressSegment;

public:
	ParkingSlot();
	virtual ~ParkingSlot();

	const unsigned int getParkingSlotId() const;

	const unsigned int getAccessSegmentId() const;
	void setAccessSegmentId(unsigned int segmentId);

	const unsigned int getEgressSegmentId() const;
	void setEgressSegmentId(unsigned int segmentId);

	double getOffset() const;
	void setOffset(double offset);

	double getLength() const;
	void setLength(double length);

	const unsigned int getCapacity() const;
	void setCapacity(unsigned int capacityPCU);

	bool isVacant() const;
	void setIsOccupied(bool occupied);

	const RoadSegment* getAccessSegment() const;
	void setAccessSegment(const RoadSegment *rdSegment);

	const RoadSegment* getEgressSegment() const;
	void setEgressSegment(const RoadSegment *rdSegment);
};

}