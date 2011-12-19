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
#ifndef DISABLE_DYNAMIC_DISPATCH
  //#define DISABLE_DYNAMIC_DISPATCH
#endif

//Use this flag to completely disable compilation with MPI. This flag basically just disables the various
//  MPI includes. It should be possible to EFFECTIVELY disable mpi by simply setting the number of
//  computers to 1, but this flag exists to allow someone without boost::mpi installed to compile the
//  Sim Mobility simulator.
//NOTE: Currently the user will still need libboost_mpi to "link" against, but when we switch to
//      cmake (or anything slightly more automated) we will disable that too.
//#ifndef SIMMOB_DISABLE_MPI
//  #define SIMMOB_DISABLE_MPI
//#endif


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




