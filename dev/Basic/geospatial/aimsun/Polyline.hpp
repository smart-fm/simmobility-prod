#pragma once

#include <string>

#include "Base.hpp"
#include "constants.h"

namespace sim_mob
{
namespace aimsun
{

//Forward declarations
class Section;


///An AIMSUN polyline (applies to a Section)
//   Polylines don't have an ID, but they still extend Base() to get access to the write flag.
class Polyline : public Base {
public:
	double xPos;
	double yPos;
	Section* section;

	Polyline() : Base(), section(NULL) {}

	//Temporaries
	int TMP_SectionId;

	//Decorated data
	double distanceFromSrc;


};


}
}
