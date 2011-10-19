/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "entities/Agent.hpp"

namespace sim_mob
{

/**
 * Role that a person may fulfill. Allows Person agents to swap out roles easily,
 * without re-creating themselves or maintaining temporarily irrelevant data.
 *
 * \note
 * For now, this class is very simplistic.
 */
class Role {
public:
	//NOTE: Don't forget to call this from sub-classes!
	Role(Agent* parent=nullptr) : parent(parent) {
	}

	/// TODO: Think through what kind of data this function might need.
	/// Frame number? Elapsed time?
	virtual void update(frame_t frameNumber) = 0;

	Agent* getParent() {
		return parent;
	}

	void setParent(Agent* parent) {
		this->parent = parent;
	}

protected:
	Agent* parent; ///<The owner of this role. Usually a Person, but I could see it possibly being another Agent.
};



}
