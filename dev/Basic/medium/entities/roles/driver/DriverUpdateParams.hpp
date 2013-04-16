/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "entities/UpdateParams.hpp"
#include "geospatial/Lane.hpp"
#include "util/DynamicVector.hpp"
#include <boost/random.hpp>
#include "util/LangHelpers.hpp"
#include "entities/signal/Signal.hpp"
#include "geospatial/Link.hpp"


namespace sim_mob
{

//Forward declarations
class Lane;


namespace medium
{
class Driver;

///Simple struct to hold parameters which only exist for a single update tick.
/// \author Melani
///NOTE: Constructor is currently implemented in Driver.cpp. Feel free to shuffle this around if you like.
struct DriverUpdateParams : public UpdateParams {
	explicit DriverUpdateParams(boost::mt19937& gen) : UpdateParams(gen), secondsInTick(0.0),
			elapsedSeconds(0.0){}

	virtual void reset(timeslice now, const Driver& owner);

	double secondsInTick;	//tickSize
	double elapsedSeconds;	//time elapsed in the current tick
};


}}
