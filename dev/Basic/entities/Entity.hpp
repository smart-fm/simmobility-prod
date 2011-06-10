/*
 * Subset of all agents. Several requirements:
 *   Must have an ID.
 *   Must have an "update" function.
 */

#pragma once

#include "../buffering/BufferedDataManager.hpp"


class Entity {
public:
	Entity(unsigned int id) : id(id) {}

	//Update time tick.
	virtual void update() = 0;

	//Set a new data manager for this entity. "isNew" is set to false to remove the data manager.
	virtual void subscribe(sim_mob::BufferedDataManager* mgr, bool isNew) = 0;

private:
	unsigned int id;

//Trivial accessors/mutators. Header-implemented
public:
	unsigned int getId() const { return id; }
};

