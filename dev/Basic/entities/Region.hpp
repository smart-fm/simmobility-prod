/*
 * Basic Region class
 */

#pragma once

#include <vector>

#include "../constants.h"

//Signals
struct Signal {
	unsigned int id;
	Signal(unsigned int id=0) : id(id) {}
};


class Region {
public:
	Region(unsigned int id=0);

	virtual void update();

private:
	unsigned int id;
	std::vector<Signal> signals;

//Trivial accessors/mutators. Header-implemented
public:
	unsigned int getId() const { return id; }
};
