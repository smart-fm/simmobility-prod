/*
 * Booking.h
 *
 *  Created on: Mar 23, 2015
 *      Author: haroldsoh
 */

#pragma once

#include "Types.hpp"
#include <istream>

namespace sim_mob{

namespace amod {

struct Booking {
public:
    enum Mode {TELEPORT, AMODTRAVEL};

    /**
     * Constructor
     * @param bookingId - Booking Id
     * @param vehicleId - Vehicle Id
     * @param customerId - Customer Id
     * @param srcPos - Source Postion
     * @param destPos - Destination Position
     * @param bkngTime - Booking time
     * @param travMode - Travel mode
     */
    Booking(int bookingId = 0, int vehicleId = 0, int customerId = 0,
	    Position srcPos = Position(),
	    Position destPos = Position(),
	    double bkngTime = 0.0,
	    amod::Booking::Mode travMode = amod::Booking::Mode::AMODTRAVEL):
		id(bookingId),
		vehId(vehicleId),
		custId(customerId),
		source(srcPos),
		destination(destPos),
		bookingTime(bkngTime),
		travelMode(travMode),
		pickupTime(0), dispatchTime(0), dropoffTime(0) {}

    /**
     * Destructor
     */
    virtual ~Booking() {}

    /// id of booking (valid bookings have > 0)
    int id;

    /// veh_id (valid veh_ids > 0)
    int vehId;

    /// cust_id (valid cust_ids > 0)
    int custId;

    /// Source position
    Position source;

    /// Desitination position
    Position destination;

    /// Booking time (in seconds)
    double bookingTime;

    /// travel mode for this booking
    Mode travelMode;

    /// the following are mainly for logging purposes (optional)
    /// dispatch time (in seconds)
    double dispatchTime;

    /// pickup time (in seconds)
    double pickupTime;

    /// dropoff time (in seconds)
    double dropoffTime;

    /**
     * operator <
     * operator< overloading
     * @param rhs - right hand side
     * @return - true if rhs booking time is greater than this
     */
    bool operator<(const Booking &rhs) const {
	    return bookingTime < rhs.bookingTime;
    }

};

/// to load in enums
inline std::istream & operator>>(std::istream & str, Booking::Mode & v) {
    unsigned int mode = 0;
    if (str >> mode)
	    v = static_cast<Booking::Mode>(mode);
    return str;
}
}

}
