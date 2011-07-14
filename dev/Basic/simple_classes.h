/**
 * \file simple_classes.h
 * Simple class definitions. If you find a class here, it will be moved to its own *.cpp,*.hpp
 * files later (or even removed).
 */
#pragma once

//namespace sim_mob {} //This is a temporary file, so it exists outside the namespace

#include "buffering/BufferedDataManager.hpp"


//Class stubs
class TripChain {
public:
	unsigned int id;
	TripChain(unsigned int id=0) : id(id) {}
};
class ChoiceSet {
public:
	unsigned int id;
	ChoiceSet(unsigned int id=0) : id(id) {}
};
class Vehicle {
public:
	unsigned int id;
	Vehicle(unsigned int id=0) : id(id) {}
};


