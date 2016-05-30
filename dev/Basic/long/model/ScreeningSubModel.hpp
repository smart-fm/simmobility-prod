//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * ScreeningSubModel.hpp
 *
 *  Created on: 19 Jan 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include <Types.hpp>
#include "HM_Model.hpp"
#include "DeveloperModel.hpp"
#include "database/entity/Unit.hpp"
#include <role/impl/HouseholdSellerRole.hpp>
#include <core/LoggerAgent.hpp>
#include <core/AgentsLookup.hpp>
#include "behavioral/PredayLT_Logsum.hpp"
#include "model/lua/LuaProvider.hpp"
#include <model/HedonicPriceSubModel.hpp>

namespace sim_mob
{
	namespace long_term
	{
		class ScreeningSubModel
		{
			public:
				ScreeningSubModel();
				virtual ~ScreeningSubModel();

				void getScreeningProbabilities(int hhId, std::vector<double> &probabilities, HM_Model *model, int day);

				BigSerial ComputeWorkPlanningArea(PlanningArea *planningAreaWork);
				BigSerial ComputeHomePlanningArea(PlanningArea *planningAreaWork, Household *household);
				void ComputeHeadOfHousehold(Household* household);
				int  GetDwellingType(int unitType);

			private:


				HM_Model *model;
				Individual* headOfHousehold;

		};
	}
}
