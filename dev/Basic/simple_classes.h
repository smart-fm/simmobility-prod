#pragma once

#include <vector>

#include "constants.h"



//Class stubs
struct Signal {
	unsigned int id;
	Signal(unsigned int id=0) : id(id) {}
};
struct Region {
	unsigned int id;
	std::vector<Signal> signals;

	Region(unsigned int id=0) : id(id) {
		for (size_t i=id*3; i<id*3+3; i++) {
			signals.push_back(Signal(i));
		}
	}
};
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
