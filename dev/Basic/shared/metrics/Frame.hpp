/* Copyright Singapore-MIT Alliance for Research and Technology */


/**
 * \file Frame.cpp
 * Centralized storage of the frame-tick parameter.
 *
 * \author LIM Fung Chai
 */


#pragma once

#include <stdint.h>

//! The type to represent frame numbers.
//!
//! Discrete-time simulation is like a stop-motion animation film.  Each simulation
//! time-step is like a frame of the film.
//!
//! Frame numbers are 32 bits unsigned ints.  If the smallest time-step is 0.1 second,
//! then this type can represent 429496729.6 seconds, or 119304.65 hours, or 4971.03 days,
//! or 13.61 years.
typedef uint32_t frame_t;
