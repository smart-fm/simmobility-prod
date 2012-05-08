/*
 * BoundaryProcessingPackage.hpp
 * Target:
 * 1. Each time, one computer should package data in boundary and transmit package to other computers.
 * 2. Each package can contain many road segment conditions.
 * 3. This Package Implement serialization
 */

#pragma once

#include <vector>

namespace sim_mob {

//Forward declarations
class Person;
class Signal;


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

