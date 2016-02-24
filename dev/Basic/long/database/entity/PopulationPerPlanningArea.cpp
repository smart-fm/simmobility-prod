/*
 * PopulationPerPlanningArea.cpp
 *
 *  Created on: 13 Aug, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/PopulationPerPlanningArea.hpp>

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

int PopulationPerPlanningArea::getPlanningAreaId() const
{
	return planningAreaId;
}

int PopulationPerPlanningArea::getPopulation() const
{
	return population;
}

int PopulationPerPlanningArea::getFloorArea() const
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










