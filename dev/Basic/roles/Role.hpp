#pragma once

#include <vector>

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

	/// TODO: Think through what kind of data this function might need.
	/// Frame number? Elapsed time?
	virtual void update() = 0;
};



}
