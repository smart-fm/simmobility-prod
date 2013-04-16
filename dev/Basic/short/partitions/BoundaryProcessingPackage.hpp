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

	std::vector<const Person*> cross_persons;
	std::vector<const Person*> feedback_persons;
	std::vector<const Signal*> boundary_signals;
};

}

