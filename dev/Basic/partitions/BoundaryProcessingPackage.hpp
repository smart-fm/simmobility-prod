/*
 * BoundaryProcessingPackage.hpp
 * Target:
 * 1. Each time, one computer should package data in boundary and transmit package to other computers.
 * 2. Each package can contain many road segment conditions.
 * 3. This Package Implement serialization
 */

#pragma once

#include <vector>
#include "entities/Person.hpp"

//TODO: Once the new signal class is stabilized, replace this include with a forward declaration:
#include "entities/signal_transitional.hpp"

namespace sim_mob {

//Forward declarations
class Person;
//class Signal_Parent;


/**
 * \author Xu Yan
 */
class BoundaryProcessingPackage {

public:
	int from_id;
	int to_id;

	std::vector<Person const*> cross_persons;
	std::vector<Person const*> feedback_persons;
	std::vector<Signal const*> boundary_signals;
};

}

