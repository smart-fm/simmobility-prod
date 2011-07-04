#pragma once

#include "../frame.hpp"
#include "../buffering/BufferedDataManager.hpp"


namespace sim_mob
{


/**
 * Base class of all agents and other "decision-making" entities.
 */
class Entity {
public:
	Entity(unsigned int id) : id(id) {}  ///<Construct an entity with an immutable ID

	/**
	 * Update function. This will be called each time tick (at the entity type's granularity; see
	 * simpleconf.hpp for more information), and will update the entity's state. During this phase,
	 * the entity may call any Buffered data type's "get" method, but may only "set" its own
	 * Buffered data. Flip is called after each update phase.
	 */
	virtual void update(frame_t frameNumber) = 0;

	/**
	 * Set a new data manager for this entity. This function is called when an entity migrates
	 * to a new worker thread, and should add (or remove, if isNew is false) all Buffered types
	 * that this entity manages.
	 *
	 * \param mgr The new or old data manager.
	 * \param isNew If true, add all Buffered types to the manager. If false, remove them.
	 */
	virtual void subscribe(sim_mob::BufferedDataManager* mgr, bool isNew) = 0;

private:
	unsigned int id;

//Trivial accessors/mutators. Header-implemented
public:
	unsigned int getId() const { return id; }
};


}

