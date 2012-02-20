/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <map>
#include <set>

#include "util/LangHelpers.hpp"
#include "util/DailyTime.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

namespace sim_mob
{

//Forward declarations
class Node;


/**
 * An activity within a trip chain. Has a location and a description.
 *
 * \author Seth N. Hetu
 */
struct TripActivity {
	std::string description;
	sim_mob::Node* location;
};



/**
 * A chain of activities.
 *
 * \author Seth N. Hetu
 */
class TripChain {
public:
	TripActivity from;
	TripActivity to;

	bool primary;
	bool flexible;

	//double startTime; //Note: Do we have a time class yet for our special format?
	sim_mob::DailyTime startTime;

	std::string mode;

	TripChain() {
		from.location = nullptr;
		to.location = nullptr;
	}

public:
#ifndef SIMMOB_DISABLE_MPI
	static void pack(PackageUtils& package, const TripChain* chain);

	static TripChain* unpack(UnPackageUtils& unpackage);
#endif
};



}
