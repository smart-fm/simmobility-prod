//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Parking.hpp"

using namespace sim_mob;
using namespace std;


Parking::Parking()
{}

Parking::~Parking()
{

	parkingDetails.clear();
}

const std::vector<const ParkingDetail *> &Parking::getParkingDetails() const
{
	return parkingDetails;
}

const ParkingDetail *Parking::getParkingDetail(unsigned int pkID) const
{
	return parkingDetails.at(pkID);
}

void Parking::addParkingDetail(const ParkingDetail *pkDet)
{
	parkingDetails.push_back(pkDet);
}