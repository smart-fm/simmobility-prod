/*
 * PopulationPerPlanningArea.cpp
 *
 *  Created on: 13 Aug, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include "database/entity/PopulationPerPlanningArea.hpp"
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

using namespace sim_mob::long_term;

PopulationPerPlanningArea::PopulationPerPlanningArea(int planningAreaId, int population, int ethnicityId, int ageCategoryId, double avgIncome, int avgHhSize, int unitType, int floorArea):
													 planningAreaId(planningAreaId), population(population), ethnicityId(ethnicityId), ageCategoryId(ageCategoryId), avgIncome(avgIncome),
													 avgHhSize(avgHhSize), unitType(unitType), floorArea(floorArea){}

PopulationPerPlanningArea::~PopulationPerPlanningArea() {}

PopulationPerPlanningArea::PopulationPerPlanningArea( const PopulationPerPlanningArea &source)
{
	planningAreaId 	= source.planningAreaId;
	population 		= source.population;
	ethnicityId 	= source.ethnicityId;
	ageCategoryId	= source.ageCategoryId;
	avgIncome		= source.avgIncome;
	avgHhSize		= source.avgHhSize;
	unitType		= source.unitType;
	floorArea		= source.floorArea;
}


PopulationPerPlanningArea& PopulationPerPlanningArea::operator=( const PopulationPerPlanningArea& source)
{
	planningAreaId 	= source.planningAreaId;
	population 		= source.population;
	ethnicityId		= source.ethnicityId;
	ageCategoryId	= source.ageCategoryId;
	avgIncome		= source.avgIncome;
	avgHhSize		= source.avgHhSize;
	unitType		= source.unitType;
	floorArea		= source.floorArea;

	return *this;
}

template<class Archive>
void PopulationPerPlanningArea::serialize(Archive & ar,const unsigned int version)
{
	ar & planningAreaId;
	ar & population;
	ar & ethnicityId ;
	ar & ethnicityId;
	ar & ageCategoryId;
	ar & avgIncome;
	ar & avgIncome;
	ar & avgHhSize;
	ar & unitType;
	ar & floorArea;

}

void PopulationPerPlanningArea::saveData(std::vector<PopulationPerPlanningArea*> &s)
{
	// make an archive
	std::ofstream ofs(filename);
	boost::archive::binary_oarchive oa(ofs);
	oa & s;
}

std::vector<PopulationPerPlanningArea*> PopulationPerPlanningArea::loadSerializedData()
{
	std::vector<PopulationPerPlanningArea*> populationPerPA;
	// Restore from saved data and print to verify contents
	std::vector<PopulationPerPlanningArea*> restored_info;
	{
		// Create and input archive
		std::ifstream ifs( "populationPerPlanningArea" );
		boost::archive::binary_iarchive ar( ifs );

		// Load the data
		ar & restored_info;
	}

	for (auto *itr :restored_info)
	{
		PopulationPerPlanningArea *popPerPA = itr;
		populationPerPA.push_back(popPerPA);
	}

	return populationPerPA;

}

int PopulationPerPlanningArea::getPlanningAreaId() const
{
	return planningAreaId;
}

int PopulationPerPlanningArea::getPopulation() const
{
	return population;
}

double PopulationPerPlanningArea::getFloorArea() const
{
	return floorArea;
}

int PopulationPerPlanningArea::getEthnicityId() const
{
	return ethnicityId;
}

int PopulationPerPlanningArea::getAgeCategoryId() const
{
	return ageCategoryId;
}

double PopulationPerPlanningArea::getAvgIncome() const
{
	return avgIncome;
}

int PopulationPerPlanningArea::getAvgHhSize() const
{
	return avgHhSize;
}

int PopulationPerPlanningArea::getUnitType() const
{
	return unitType;
}










