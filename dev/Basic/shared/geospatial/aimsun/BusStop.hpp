//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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

/**
 * aimsun class for bus stop -> segment mapping.
 */
class BusStop : public Base {
public:
	std::string  bus_stop_no;
	std::string status;
	std::string  lane_type;
	std::string  road_name;
	Section* atSection;

	double xPos;
	double yPos;

	BusStop() : Base(), atSection(nullptr), bus_stop_no(""), status(""), lane_type(""), road_name(""),
			xPos(0.0), yPos(0.0), TMP_AtSectionID(0), TMP_RevSectionID(0), TMP_AtLaneID(0)
	{}

	//Placeholders
	int TMP_AtSectionID;
	int TMP_RevSectionID; // contains non zero value only for bus terminus stops
	int TMP_AtLaneID;

};

class BusStopSG : public Base {
public:
	std::string  bus_stop_no;
	std::string stop_code;
	std::string  stop_name;
	std::string stop_lat;
	std::string stop_lon;
	std::string section_id;
	int aimsun_section;
	Section* atSection;

	double xPos;
	double yPos;

	BusStopSG() : Base(), atSection(nullptr) {}
};


}
}
