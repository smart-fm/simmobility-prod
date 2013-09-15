//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


/**
 * \file Frame.cpp
 * Centralized storage of the frame-tick parameter.
 *
 * \author LIM Fung Chai
 */


#pragma once

#include <stdint.h>


/**
 * A simple class representing a timeslice of the simulation.
 *
 * For a discrete time-stepped simulation like Sim Mobility, a single timeslice is updated
 * all at once (conceptually). Thus, we can say "frame 10" is updated all at once. This
 * "frame" is similar to a frame in film, and can be seen as a static view of the world at that given time "tick".
 *
 * In addition to the current frame, the current timeslice also stores the offset in ms from the start of the
 * simulation. This is easy to compute externally, but since this single computation is performed so many times by
 * so many Agents, we hope to minimize errors by consolidating it here. A timeslice is constant once constructed.
 *
 * Note that, for 32-bit unsigned integers, the maximum simulation time (based on the ms value) is just over 49
 * days. This does not depend on the time-step of the simulation.
 */
class timeslice {
public:
	timeslice(uint32_t frame, uint32_t ms) : frame_(frame), ms_(ms) {}

	uint32_t frame() const { return frame_; }
	uint32_t ms() const { return ms_; }

private:
	uint32_t frame_;
	uint32_t ms_;
};

