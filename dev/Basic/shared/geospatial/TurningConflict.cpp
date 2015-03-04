//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <sstream>
#include <geospatial/TurningConflict.hpp>

using namespace sim_mob;

sim_mob::TurningConflict::TurningConflict() :
		conflictId(""),dbId(-1),first_turning(""),second_turning(""),first_cd(-1),second_cd(-1),
		firstTurning(nullptr),secondTurning(nullptr),criticalGap(0)
{


}

sim_mob::TurningConflict::TurningConflict(const TurningConflict& tc) :
		dbId(tc.dbId),first_turning(tc.first_turning),second_turning(tc.second_turning),
		first_cd(tc.first_cd),second_cd(tc.second_cd),
		firstTurning(tc.firstTurning),secondTurning(tc.secondTurning),criticalGap(tc.criticalGap)
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

void TurningConflict::setSecondTurning(TurningSection* secondTurning)
{
	this->secondTurning = secondTurning;
}

TurningSection* TurningConflict::getSecondTurning() const
{
	return secondTurning;
}

void TurningConflict::setFirstTurning(TurningSection* firstTurning)
{
	this->firstTurning = firstTurning;
}

TurningSection* TurningConflict::getFirstTurning() const
{
	return firstTurning;
}

void TurningConflict::setConflictId(std::string conflictId)
{
	this->conflictId = conflictId;
}

std::string TurningConflict::getConflictId() const
{
	return conflictId;
}

void TurningConflict::setSecond_cd(double second_cd)
{
	this->second_cd = second_cd;
}

double TurningConflict::getSecond_cd() const
{
	return second_cd;
}

void TurningConflict::setFirst_cd(double first_cd)
{
	this->first_cd = first_cd;
}

double TurningConflict::getFirst_cd() const
{
	return first_cd;
}

void TurningConflict::setSecond_turning(std::string second_turning)
{
	this->second_turning = second_turning;
}

std::string TurningConflict::getSecond_turning() const
{
	return second_turning;
}

void TurningConflict::setFirst_turning(std::string first_turning)
{
	this->first_turning = first_turning;
}

std::string TurningConflict::getFirst_turning() const
{
	return first_turning;
}

void TurningConflict::setDbId(int dbId)
{
	this->dbId = dbId;
}

int TurningConflict::getDbId() const
{
	return dbId;
}

void TurningConflict::setCriticalGap(double criticalGap)
{
	this->criticalGap = criticalGap;
}

double TurningConflict::getCriticalGap() const
{
	return criticalGap;
}
