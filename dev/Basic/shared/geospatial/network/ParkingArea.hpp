//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>

namespace sim_mob
{

class ParkingSlot;

/**
 * Represents a collection of individual road side parking spaces
 * \author Neeraj
 */
class ParkingArea
{
private:
	/**The identifer for the parking area*/
	unsigned int areaId;
	
	/**The collection of parking slots that belong to this area*/
	std::vector<const ParkingSlot *> parkingSlots;

public:
	ParkingArea(int id);
	~ParkingArea();

	const unsigned int getParkingAreaId() const;
	void setParkingAreaId(unsigned int areaId);

	const std::vector<const ParkingSlot *>& getParkingSlots() const;
	const ParkingSlot* getParkingSlot(unsigned int index) const;

	/**
	 * Adds a parking slot to the parking area
	 * @param pkSlot The pointer to the parking slot object to be added
	 */
	void addParkingSlot(const ParkingSlot *pkSlot);
};

}