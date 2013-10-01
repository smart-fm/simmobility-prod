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


///An AIMSUN "turning" (aggregate of Lane Connectors)
/// \author Seth N. Hetu
class Turning : public Base {
public:
	std::pair<int, int> fromLane;
	std::pair<int, int> toLane;
	sim_mob::aimsun::Section* fromSection;
	sim_mob::aimsun::Section* toSection;

	Turning() : Base(), fromSection(nullptr), toSection(nullptr), TMP_FromSection(0), TMP_ToSection(0)
	{}

	//Temporaries
	int TMP_FromSection;
	int TMP_ToSection;
};


}
}
