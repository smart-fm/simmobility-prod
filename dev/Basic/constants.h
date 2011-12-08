/* Copyright Singapore-MIT Alliance for Research and Technology */

/**
 * \file constants.h
 * Constant definitions. Most of these will become configurable parameters later.
 * \note Re-name to constants.hpp if we later decide to keep some constants; the .h extension
 * is used to indicate a temporary file.
 *
 * \par
 * ~Seth
 */


#pragma once

#include <cmath>

//namespace sim_mob {} //This is a temporary file, so it exists outside the namespace


//Should we use dynamic dispatch, or force all Agents to start at time tick zero?
// This is a workaround for current bugs; it will be removed later.
#define DISABLE_DYNAMIC_DISPATCH


//Sizes of workgroups. (Note that enums are allowed to overlap values)
enum WORKGROUP_SIZES {
	WG_TRIPCHAINS_SIZE = 4,       ///<Number of trip chain workers in group.
	WG_CREATE_AGENT_SIZE = 3,     ///<Number of agent creation workers in group.
	WG_CHOICESET_SIZE = 6,        ///<Number of choice set workers in group.
	WG_AGENTS_SIZE = 5,           ///<Number of agent workers in group.
	WG_SIGNALS_SIZE = 2,          ///<Number of signal workers in group.
	WG_SHORTEST_PATH_SIZE = 10,   ///<Number of shortest path workers in group.
};


//Note: Un-named namespace used to avoid multiple definitions error.
//      Of course, these functions need a common place eventually, like a "utils" hpp/cpp
namespace {


/**
 * Declaration of a "trivial" function, which does nothing and returns a trivial conditional.
 * Used to indicate future functionality.
 */
inline bool trivial(unsigned int id)
{
	return id%2==0;
}



}




