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
	/**The distance of the parking slot from the start of the segment*/
	double offset;

	/**The length of the parking slot in metre*/
	double length;

	/**Indicates whether the parking slot is occupied*/
	bool isOccupied;

	/**The parent road segment*/
	const RoadSegment *parentSegment;

public:
	ParkingSlot();
	virtual ~ParkingSlot();

	double getOffset() const;
	void setOffset(double offset);

	double getLength() const;
	void setLength(double length);

	bool isVacant() const;
	void setIsOccupied(bool occupied);

	const RoadSegment* getParentSegment() const;
	void setParentSegment(const RoadSegment *rdSegment);
};

}