/**
 * \file simple_classes.h
 * Simple class definitions. If you find a class here, it will be moved to its own *.cpp,*.hpp
 * files later (or even removed).
 */
#pragma once

//namespace sim_mob {} //This is a temporary file, so it exists outside the namespace

//Class stubs
struct TripChain {
	unsigned int id;
	TripChain(unsigned int id=0) : id(id) {}
};
struct ChoiceSet {
	unsigned int id;
	ChoiceSet(unsigned int id=0) : id(id) {}
};
struct Vehicle {
	unsigned int id;
	Vehicle(unsigned int id=0) : id(id) {}
};



