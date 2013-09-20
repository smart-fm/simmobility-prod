//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/misc/TripChain.hpp"

namespace sim_mob
{

class Node;
class Person;

///Type of event that can be "Pending"
enum KNOWN_EVENT_TYPES {
	TRIP_START,      ///start of a whole trip.
	TRIP_END,  ///End of a whole trip.
	ACTIVITY_START,   ///start of an activity during the trip.
	ACTIVITY_END,    ///end of an activity during the trip
};

/**
 * Lightweight entity container. Used to hold acitivity waiting to be scheduled.
 *
 * \author vuvinhan
 *
 * \note
 */
struct PendingEvent {
	//Make an activity.
	explicit PendingEvent(KNOWN_EVENT_TYPES type);

	//Helper: make a raw entity.
	explicit PendingEvent(KNOWN_EVENT_TYPES type, Node* location, unsigned start);

	KNOWN_EVENT_TYPES type;  ///<Event type.
	Node* location;             ///<Event location
	//Person* rawAgent;         ///<The agent who conducts the Event
	unsigned int start;       ///<Event start time
	unsigned int end;       ///<Event end time
	//int manualID;             ///<Manual ID for this entity. If -1, it is assigned an ID
};

}

