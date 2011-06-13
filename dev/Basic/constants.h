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

//namespace sim_mob {} //This is a temporary file, so it exists outside the namespace

const unsigned int WG_TRIPCHAINS_SIZE = 4;       ///<Number of trip chain workers in group.
const unsigned int WG_CREATE_AGENT_SIZE = 3;     ///<Number of agent creation workers in group.
const unsigned int WG_CHOICESET_SIZE = 6;        ///<Number of choice set workers in group.
const unsigned int WG_VEHICLES_SIZE = 5;         ///<Number of vehicle workers in group.
const unsigned int WG_AGENTS_SIZE = 5;           ///<Number of agent workers in group.
const unsigned int WG_SIGNALS_SIZE = 2;          ///<Number of signal workers in group.
const unsigned int WG_SHORTEST_PATH_SIZE = 10;   ///<Number of shortest path workers in group.



/**
 * Declaration of a "trivial" function, which does nothing and returns a trivial conditional.
 * Used to indicate future functionality.
 */
bool trivial(unsigned int id);



