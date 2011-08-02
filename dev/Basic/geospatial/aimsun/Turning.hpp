#pragma once

#include <string>

#include "Base.hpp"
#include "Section.hpp"
#include "../../constants.h"

namespace sim_mob
{
namespace aimsun
{

///An AIMSUN "turning" (aggregate of Lane Connectors)
class Turning : public Base {
public:
	std::pair<int, int> fromLane;
	std::pair<int, int> toLane;
	sim_mob::aimsun::Section* fromSection;
	sim_mob::aimsun::Section* toSection;

	Turning() : fromSection(nullptr), toSection(nullptr) {}

	//Temporaries
	int TMP_FromSection;
	int TMP_ToSection;
};


}
}
