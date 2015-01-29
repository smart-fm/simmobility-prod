//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include "TurningSection.hpp"

namespace sim_mob
{

class TurningSection;

class TurningConflict {
public:
	TurningConflict();
	TurningConflict(const TurningConflict& tc);
public:
	int dbId;
	std::string first_turning;
	std::string second_turning;
	double first_cd;
	double second_cd;
        
	std::string conflictId;
	TurningSection *firstTurning;
	TurningSection *secondTurning;
};

};// end namespace

