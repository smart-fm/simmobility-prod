//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <map>

namespace sim_mob
{

class ParkingDetail;

/**
 * Represents a collection of individual parking
 *
 */
class Parking
{

private:
    //unsigned int parking_id;
    std::vector<const ParkingDetail *> parkingDetails;
    //std::map<const unsigned int parking_id, const ParkingDetail *> MapparkingDetails;
public:
public:
    Parking();

    ~Parking();

    /*
	 * function return Vector of complete set of Parking
	 */
    const std::vector<const ParkingDetail *> &getParkingDetails() const;

    /**
	* Returns the parking detail for the passed parking id
	* @param index the index of the requested parking slot
	* @return pointer to the parking slot in the parking area at the given index
	*/
    const ParkingDetail *getParkingDetail(unsigned int pkID) const;

    void addParkingDetail(const ParkingDetail *);
};
}