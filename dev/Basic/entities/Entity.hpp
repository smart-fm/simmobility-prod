/*
 * Subset of all agents. Several requirements:
 *   Must have an ID.
 *   Must have an "update" function.
 */

#pragma once


class Entity {
public:
	Entity(unsigned int id) : id(id) {}
	virtual void update() = 0;

private:
	unsigned int id;

//Trivial accessors/mutators. Header-implemented
public:
	unsigned int getId() const { return id; }
};

