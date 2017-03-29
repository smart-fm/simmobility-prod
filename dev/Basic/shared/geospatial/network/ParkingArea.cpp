//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <stdexcept>
#include "ParkingArea.hpp"

using namespace sim_mob;
using namespace std;

ParkingArea::ParkingArea(int id) : areaId(id) 
{
}

ParkingArea::~ParkingArea()
{
	//The objects will be deleted by the destructor to the road segment, as the parking slots are stored 
	//as obstacles
	parkingSlots.clear();
}

const unsigned int ParkingArea::getParkingAreaId() const
{
	return areaId;
}

void ParkingArea::setParkingAreaId(unsigned int areaId)
{
	this->areaId = areaId;
}

const std::vector<const ParkingSlot *>& ParkingArea::getParkingSlots() const
{
	return parkingSlots;
}

const ParkingSlot* ParkingArea::getParkingSlot(unsigned int index) const
{
	try
	{
		return parkingSlots.at(index);
	}
	catch (out_of_range &ex)
	{
		return nullptr;
	}
}

void ParkingArea::addParkingSlot(const ParkingSlot *pkSlot)
{
	parkingSlots.push_back(pkSlot);
}