//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/random.hpp>
#include "entities/UpdateParams.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Link.hpp"
#include "util/DynamicVector.hpp"
#include "util/LangHelpers.hpp"

namespace sim_mob
{

//Forward declarations
class Lane;

namespace medium
{
class Driver;

/**
 * Simple struct to hold parameters which only exist for a single update tick.
 * \author Melani
 */
struct DriverUpdateParams : public UpdateParams {
	DriverUpdateParams() : UpdateParams(), secondsInTick(0.0),
			elapsedSeconds(0.0) {}
	explicit DriverUpdateParams(boost::mt19937& gen) : UpdateParams(gen), secondsInTick(0.0),
			elapsedSeconds(0.0){}

	virtual void reset(timeslice now, const Driver& owner);

	double secondsInTick;	//tickSize
	double elapsedSeconds;	//time elapsed in the current tick
};
}
}
