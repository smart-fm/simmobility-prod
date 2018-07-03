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
#include <sstream>  //stringstream
#include <stdexcept>

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
	timeslice(uint32_t frame, uint32_t ms) : frame_(frame), ms_(ms) {};
	timeslice (const timeslice& t):frame_(t.frame()), ms_(t.ms()){};

	uint32_t frame() const { return frame_; }
	uint32_t ms() const { return ms_; }
	double getSeconds() const { return ms_ / 1000.0;}

	bool operator==(const timeslice& other) const
	{
		if (ms_ == other.ms() )
		{
#ifndef NDEBUG
			if (frame_ != other.frame() )
			{
				std::stringstream msg; msg<<"Error: in ms "<< ms_ <<"=="<<other.ms()<<", but in frames "<<
					frame_<<">="<<other.frame();
				throw std::runtime_error(msg.str());
			}
#endif
			return true;
		}
		return false;
	}

	bool operator!=(const timeslice& other) const
	{
		if (frame_ != other.frame() )
		{
#ifndef NDEBUG
			if ( ms_ == other.ms() )
			{
				std::stringstream msg; msg<<"Error: in frames "<<frame_<<"!="<<other.frame()<<
				", but in ms "<< ms_ <<"=="<< other.ms();
				throw std::runtime_error(msg.str() );
			}
#endif
			return true;
		}
		else return false;
	}

	bool operator<(const timeslice& other) const
	{
		if (ms_ < other.ms() )
		{
#ifndef NDEBUG
			if (frame_ >= other.frame() )
			{
				std::stringstream msg; msg<< "Error: in milliseconds "<< ms_ <<"<" << other.ms() << ", but in frames "<<
					frame_ <<">="<< other.frame();
				throw std::runtime_error(msg.str() );
			}
#endif
			return true;
		}
		return false;
	}

private:
	uint32_t frame_;
	uint32_t ms_
	;
};

std::ostream& operator<<(std::ostream& strm, const timeslice& ts);
