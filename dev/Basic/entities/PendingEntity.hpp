/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

namespace sim_mob
{

enum KNOWN_ENTITY_TYPES {
	ENTITY_DRIVER,
	ENTITY_PEDESTRIAN,
	ENTITY_BUSDRIVER
};

/**
 * Lightweight entity container. Used to hold Entities waiting to be scheduled.
 */
struct PendingEntity {
	KNOWN_ENTITY_TYPES type;
	Node* origin;
	Node* dest;
	unsigned int start;
};

}

