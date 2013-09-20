//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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
