//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/random.hpp>
#include "entities/UpdateParams.hpp"
#include "metrics/Frame.hpp"

namespace sim_mob
{
namespace medium
{
/**
 * Simple struct to hold driver parameters which only exist for a single time tick.
 * \author Melani
 */
struct DriverUpdateParams : public UpdateParams {
	DriverUpdateParams()
	: UpdateParams(), secondsInTick(0.0), elapsedSeconds(0.0) {}
	explicit DriverUpdateParams(boost::mt19937& gen)
	: UpdateParams(gen), secondsInTick(0.0), elapsedSeconds(0.0){}

	/**
	 * resets this update params.
	 * @param now current timeslice in which reset is called
	 */
	virtual void reset(timeslice now);

	/**tickSize in seconds*/
	double secondsInTick;
	/**time elapsed in the current tick (in seconds)*/
	double elapsedSeconds;
};
}
}
