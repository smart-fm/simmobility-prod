#pragma once

#include <string>

#include "Base.hpp"
#include "Section.hpp"
#include "../../constants.h"

namespace sim_mob
{
namespace aimsun
{

///An AIMSUN polyline (applies to a Section)
//   Polylines don't have an ID
class Polyline {//: public Base {
public:
	double xPos;
	double yPos;
	Section* section;

	Polyline() : section(NULL) {}

	//Temporaries
	int TMP_SectionId;


};


}
}
