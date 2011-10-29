/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <string>
//#include <cmath>
//#include <map>

//#include "util/LangHelpers.hpp"

#include "Base.hpp"

namespace sim_mob
{

namespace aimsun
{

//Forward declarations
class Node;



///An activity within a trip chain
struct TripActivity {
	std::string description;
	Node* location;

	//Placeholder
	int TMP_locationNodeID;
};



///A trip chain. Not technically part of AIMSUN; we may have to rename this folder later.
class TripChain : public Base {
public:
	TripActivity from;
	TripActivity to;

	bool primary;
	bool flexible;

	double startTime; //Note: Do we have a time class yet for our special format?

	std::string mode;

	TripChain() : Base() {
		from.location = nullptr;
		to.location = nullptr;
	}

	//Reference to saved object
	//sim_mob::TripChain* generatedTC;

};


}
}
