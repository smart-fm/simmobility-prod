/*
 * PopulationPerPlanningArea.hpp
 *
 *  Created on: 13 Aug, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class PopulationPerPlanningArea
		{
		public:
			PopulationPerPlanningArea(int planning_area_id = 0, int population = 0);
			virtual ~PopulationPerPlanningArea();

			PopulationPerPlanningArea( const PopulationPerPlanningArea & source);
			PopulationPerPlanningArea& operator=(const PopulationPerPlanningArea & source);

			int getPlanningAreaId() const;
			int getPopulation() const;

			int population;
			int planningAreaId;

		};
	}
}

