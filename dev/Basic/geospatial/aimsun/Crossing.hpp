/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <string>
#include <cmath>


#include "util/LangHelpers.hpp"

#include "Base.hpp"


namespace sim_mob
{

//Forward declarations


namespace aimsun
{

//Forward declarations
class Section;


///An AIMSUN road intersection or segment intersection.
//   Crossings don't have an ID, but they still extend Base() to get access to the write flag.
/// \author Seth N. Hetu
class Crossing : public Base {
public:
	int laneID;
	std::string laneType;
	Section* atSection;

	double xPos;
	double yPos;

	Crossing() : Base(), atSection(nullptr) {}

	//Placeholders
	int TMP_AtSectionID;

	//Decorated data
	Node* atNode; //Which Node this crossing is relevant for.
};


}
}
