/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <map>
#include <set>
#include <string>

#include "util/LangHelpers.hpp"
#include "util/DailyTime.hpp"
#include "geospatial/Node.hpp"

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
	Node * location;

	//added to handle start time and end time of activity
	//by Jenny (07 Jun, 2012)
	unsigned int startTime;
	unsigned int endTime;
};

/*
 * A subtrip within a trip train. Has startnode, endnode, mode
 * Added by Jenny
 */
struct SubTrip {
	TripActivity from;
	TripActivity to;
	std::string mode;

	sim_mob::DailyTime startTime;
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

	std::string mode; //primamry mode

	//added to handle activity in trip chain
	//by Jenny (07 Jun, 2012)
	std::vector<TripActivity*> activities;
	std::vector<SubTrip*> subTrips;

	TripChain() {
		from.location = nullptr;
		to.location = nullptr;
	}

	TripChain(TripActivity from, TripActivity to, std::string mode, std::vector<TripActivity*> activities, std::vector<SubTrip*> subTrips)
		: from(from), to(to), mode(mode),activities(activities), subTrips(subTrips){}

public:
#ifndef SIMMOB_DISABLE_MPI
	static void pack(PackageUtils& package, const TripChain* chain);

	static TripChain* unpack(UnPackageUtils& unpackage);
#endif
};



}
