/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <map>
#include <string>
#include <stdexcept>

#include "util/LangHelpers.hpp"
#include "util/DailyTime.hpp"

namespace sim_mob {


/**
 * A granularity, which can be represented as "x hours", "x minutes", and so forth.
 * The ms() function returns the most often-accessed format; we may choose to add
 * hours(), minutes(), and so forth later.
 */
class Granularity {
public:
	Granularity(int amount=0, const std::string& units="ms");
	int ms() { return ms_; }

private:
	int ms_;
};





/**
 * Collection of various Simulation-level settings.
 */
struct Simulation {
	Simulation() : leadingVehReactTime(nullptr), subjectVehReactTime(nullptr), vehicleGapReactTime(nullptr) {}

	Granularity baseGranularity;
	Granularity totalRuntime;
	Granularity totalWarmup;
	DailyTime startTime;

	Granularity agentGranularity;
	Granularity signalGranularity;

	ReactionTimeDist* leadingVehReactTime;
	ReactionTimeDist* subjectVehReactTime;
	ReactionTimeDist* vehicleGapReactTime;

	//TODO: Geospatial loaders.
	//TODO: Agent loaders.

};

}
