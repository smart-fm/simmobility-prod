

#include "Parking.hpp"

using namespace sim_mob;
using namespace std;



Parking::Parking(){}

Parking::~Parking()
{

    parkingDetails.clear();
}

const std::vector<const ParkingDetail *>& Parking::getParkingDetails() const
{
    return parkingDetails;
}

const ParkingDetail* Parking::getParkingDetail(unsigned int pkID) const
{
    return parkingDetails.at(pkID);
}

void Parking::addParkingDetail(const ParkingDetail *pkDet)
{
    parkingDetails.push_back(pkDet);
}