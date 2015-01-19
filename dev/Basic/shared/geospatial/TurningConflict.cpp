//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <geospatial/TurningConflict.hpp>

sim_mob::TurningConflict::TurningConflict() :
		dbId(-1),first_turning(""),second_turning(""),first_cd(-1),second_cd(-1),
		firstTurning(nullptr),secondTurning(nullptr)
{


}


sim_mob::TurningConflict::TurningConflict(const TurningConflict& tc) :
		dbId(tc.dbId),first_turning(tc.first_turning),second_turning(tc.second_turning),
		first_cd(tc.first_cd),second_cd(tc.second_cd),
		firstTurning(tc.firstTurning),secondTurning(tc.secondTurning)
{


}
