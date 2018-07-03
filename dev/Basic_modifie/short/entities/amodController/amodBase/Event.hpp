/*
 * Event.hpp
 *
 *  Created on: Mar 27, 2015
 *      Author: haroldsoh
 */

#pragma once

#include <string>
#include <vector>

namespace sim_mob{

namespace amod {

enum EventType {
    EVENT_DISPATCH,
    EVENT_MOVE,
    EVENT_ARRIVAL,
    EVENT_PICKUP,
    EVENT_DROPOFF,
    EVENT_LOCATION_VEHS_SIZE_CHANGE,
    EVENT_LOCATION_CUSTS_SIZE_CHANGE,
    EVENT_BOOKING_CANNOT_BE_SERVICED,
    EVENT_TELEPORT,
    EVENT_TELEPORT_ARRIVAL,
    EVENT_BOOKING_RECEIVED,
    EVENT_BOOKING_SERVICED,
    EVENT_REBALANCE,
    EVENT_VEH_SIMULATION_ERROR,
};

struct Event {
    /**
     * Constructor
     * @param evntType - Event type
     * @param evntId - Event Id
     * @param evntName - Event Name
     * @param evntTime - Time of the event
     * @param entIds - Entity ids
     */
    Event(EventType evntType, long long evntId, const std::string& evntName, double evntTime, std::vector<int> entIds) :
	    type(evntType), id(evntId), name(evntName), t(evntTime), entityIds(entIds) {}

    /**
     * Constructor
     */
    Event(): id(0) {}

    /**
     * Destructor
     */
    ~Event() {}

    /// Type of the event
    EventType type;

    /// specific event identifier
    long long id;

    /// Event name
    std::string name;

    /// Event time
    double t;

    /// Entity ids
    std::vector<int> entityIds;

};

}

}
