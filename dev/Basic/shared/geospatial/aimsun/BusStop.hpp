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
	std::string  bus_stop_no;
	std::string status;
	std::string  lane_type;
	std::string  road_name;
	Section* atSection;

	double xPos;
	double yPos;



	BusStop() : Base(), atSection(nullptr) {}

	//Placeholders
	int TMP_AtSectionID;
	int TMP_AtLaneID;

};


}
}
