/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "PendingEvent.hpp"
#include "entities/Person.hpp"
#include "entities/misc/TripChain.hpp"

sim_mob::PendingEvent::PendingEvent(KNOWN_EVENT_TYPES type)
	: type(type), location(nullptr), start(0), end(0)
{
}

sim_mob::PendingEvent::PendingEvent(KNOWN_EVENT_TYPES type, Node* location, unsigned start)
	: type(type), location(location), start(start)
{
}
