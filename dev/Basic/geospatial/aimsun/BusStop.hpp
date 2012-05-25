/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <string>
#include <cmath>

#include "util/LangHelpers.hpp"

#include "Base.hpp"

namespace sim_mob
{
//

//Forward declarations


namespace aimsun
{
//
//Forward declarations
class Section;
class Lane;
class Polyline;


class BusStop : public Base {
public:
	int bus_stop_id;
	Section* atSection;
	Lane* atLane;

	double xPos;
	double yPos;

	BusStop() : Base(), atSection(nullptr) {}

	//Placeholders
	int TMP_AtSectionID;
	int TMP_AtLaneID;

};


}
}
