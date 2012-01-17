/*
 * BoundaryProcessingPackage.hpp
 * Target:
 * 1. Each time, one computer should package data in boundary and transmit package to other computers.
 * 2. Each package can contain many road segment conditions.
 * 3. This Package Implement serialization
 */

#pragma once

#ifndef SIMMOB_DISABLE_MPI
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <vector>
#include "entities/Person.hpp"
#include "entities/Signal.hpp"

namespace sim_mob {

/**
 * \author Xu Yan
 */
class BoundaryProcessingPackage {

public:
	std::vector<Person const*> cross_persons;
	std::vector<Person const*> feedback_persons;
	std::vector<Signal const*> boundary_signals;
};

}
#endif
