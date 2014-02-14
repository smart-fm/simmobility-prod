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
