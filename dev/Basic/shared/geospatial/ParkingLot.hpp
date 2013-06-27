//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>


namespace sim_mob
{


//Forward declarations
class RoadSegment;


/**
 * A simple parking lot. All parking lots must have their own (small) RoadSegments which lead off
 * from an Intersection and then to the center of this ParkingLot. This allows cars leaving the lot
 * to be queued up while waiting to exit. Small ParkingLots will have one RoadSegment which
 * functions as both the entrance and the exit.
 *
 * \author Seth N. Hetu
 */
class ParkingLot {
public:
	///Retrieve entrances to this ParkingLot.
	const std::vector<const RoadSegment*>& getEntrances() { return entrances; }

	///Retrieve exits to this ParkingLot.
	const std::vector<const RoadSegment*>& getExits() { return exits; }


	///Time when charging begins. Encoded as hour*100 + minutes, so 8:30AM is 830.
	///Parking is free outside the "start" to "end" period.
	unsigned int start_of_charging_period;

	///Time when charging ends. See also: start_of_chargin_period
    unsigned int end_of_charging_period;

    ///If a vehicle leaves the parking lot before the grace period is up,
    /// it is not charged for parking.
    unsigned int grace_period_in_minutes;

    ///Maximum number of cars this parking lot can hold.
    unsigned int capacity;

    ///Current number of cars parked in this lot.
    unsigned int occupancy;

    //Cost of parking in this lot.
    unsigned int charge_in_cents_per_half_hour;




private:
	std::vector<const RoadSegment*> entrances;

	std::vector<const RoadSegment*> exits;
};





}
