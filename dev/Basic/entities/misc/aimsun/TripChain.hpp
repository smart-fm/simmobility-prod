/* Copyright Singapore-MIT Alliance for Research and Technology */

//
//NOTE:
//
// I realize that the naming is off; a Trip Chain isn't part of the AIMSUN data. However, all the
//   database-specific items are in an "aimsun" folder AND in the "aimsun" namespace. If you are
//   going to rename this folder, make sure you consider what to do about "aimsun" database data versus
//   simmobility database data.
//


#pragma once

#include <string>

#include "geospatial/aimsun/Base.hpp"
#include "util/DailyTime.hpp"

namespace sim_mob
{

//Forward declarations
class TripChain;

namespace aimsun
{

//Forward declarations
class Node;



///An activity within a trip chain
/// \author Seth N. Hetu
struct TripActivity {
	std::string description;
	Node* location;

	//Placeholder
	int TMP_locationNodeID;

	//added to handle start time and end time of activity
	//by Jenny (07 Jun, 2012)
	sim_mob::DailyTime startTime;
	sim_mob::DailyTime endTime;
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

///A trip chain. Not technically part of AIMSUN; we may have to rename this folder later.
/// \author Seth N. Hetu
class TripChain /*: public Base*/ {
public:
	TripActivity from;
	TripActivity to;

	bool primary;
	bool flexible;

	sim_mob::DailyTime startTime;

	std::string mode;

	//added to handle activity in trip chain
		//by Jenny (07 Jun, 2012)
	std::vector<TripActivity*> activities;
	std::vector<SubTrip*> subTrips;

	TripChain() /*: Base()*/ {
		from.location = nullptr;
		to.location = nullptr;
	}

	TripChain(TripActivity from, TripActivity to, std::string mode, std::vector<TripActivity*> activities, std::vector<SubTrip*> subTrips)
			: from(from), to(to), mode(mode),activities(activities), subTrips(subTrips){}

	//Placeholder
	std::string TMP_startTimeStr;

	//Unused
	int EMPTY_activityID;

	//Reference to saved object
	sim_mob::TripChain* generatedTC;

};


}
}
