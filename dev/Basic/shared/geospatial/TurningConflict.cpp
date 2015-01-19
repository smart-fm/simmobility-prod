//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <sstream>
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
		std::stringstream out("");
		out<<tc.dbId;
		conflictId = out.str();

		std::stringstream trimmer;
		trimmer << first_turning;
		first_turning.clear();
		trimmer >> first_turning;

		trimmer.clear();
		trimmer << second_turning;
		second_turning.clear();
		trimmer >> second_turning;

}
