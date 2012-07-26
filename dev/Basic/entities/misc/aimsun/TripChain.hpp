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
};

class BusSchedule
{
public:
	std::string tripid;
	sim_mob::DailyTime startTime;
	std::string TMP_startTimeStr;
	BusSchedule() {}
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

	TripChain() /*: Base()*/ {
		from.location = nullptr;
		to.location = nullptr;
	}

	//Placeholder
	std::string TMP_startTimeStr;

	//Unused
	int EMPTY_activityID;

	//Reference to saved object
	sim_mob::TripChain* generatedTC;

};


}
}
