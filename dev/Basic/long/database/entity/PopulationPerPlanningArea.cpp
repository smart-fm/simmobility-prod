/*
 * PopulationPerPlanningArea.cpp
 *
 *  Created on: 13 Aug, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/PopulationPerPlanningArea.hpp>

using namespace sim_mob::long_term;

PopulationPerPlanningArea::PopulationPerPlanningArea(int planningAreaId, int population): planningAreaId(planningAreaId), population(population) {}

PopulationPerPlanningArea::~PopulationPerPlanningArea() {}

PopulationPerPlanningArea::PopulationPerPlanningArea( const PopulationPerPlanningArea &source)
{
	planningAreaId = source.planningAreaId;
	population = source.population;
}


PopulationPerPlanningArea& PopulationPerPlanningArea::operator=( const PopulationPerPlanningArea& source)
{
	planningAreaId = source.planningAreaId;
	population = source.population;

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
