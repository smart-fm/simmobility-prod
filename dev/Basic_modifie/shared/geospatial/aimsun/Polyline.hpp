//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>

#include "Base.hpp"

namespace sim_mob {
namespace aimsun {

//Forward declarations
class Section;


///An AIMSUN polyline (applies to a Section)
///   Polylines don't have an ID, but they still extend Base() to get access to the write flag.
/// \author Seth N. Hetu
class Polyline : public Base {
public:
	double xPos;
	double yPos;
	Section* section;

	Polyline() : Base(), xPos(0), yPos(0), section(NULL), TMP_SectionId(0), distanceFromSrc(0)
	{}

	//Temporaries
	int TMP_SectionId;

	//Decorated data
	double distanceFromSrc;


};


}
}
