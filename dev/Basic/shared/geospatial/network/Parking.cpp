//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <stdexcept>
#include "Parking.hpp"
#include "ParkingDetail.hpp"
#include "RoadNetwork.hpp"

using namespace sim_mob;
using namespace std;
std::map<unsigned int, ParkingDetail *>Parking::parkingDetails;

Parking::Parking()
{

}

Parking::~Parking()
{

	parkingDetails.clear();
}

const std::map<unsigned int, ParkingDetail *> &Parking::getParkingDetails()
{

    const sim_mob::RoadNetwork *roadNetwork = sim_mob::RoadNetwork::getInstance();
    parkingDetails= roadNetwork->getMapOfIdVsParkingDetails();
    return parkingDetails;
}

const ParkingDetail* Parking::getParkingDetail(unsigned int pkID) const
{
	return parkingDetails.at(pkID);
}

void Parking::addParkingDetail( ParkingDetail *pkDet)
{
	parkingDetails.insert(std::make_pair(pkDet->getParkingID(),pkDet));
}