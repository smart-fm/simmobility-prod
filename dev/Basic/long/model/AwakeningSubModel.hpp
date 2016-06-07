//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * AwakeningSubModel.hpp
 *
 *  Created on: 1 Feb 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once
#include "database/entity/ExternalEvent.hpp"
#include "model/HM_Model.hpp"
#include "agent/impl/HouseholdAgent.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class AwakeningSubModel
		{
		public:
			AwakeningSubModel();
			virtual ~AwakeningSubModel();

			void InitialAwakenings(HM_Model *model, Household *household, HouseholdAgent *agent, int day);
			std::vector<ExternalEvent> DailyAwakenings(int day, HM_Model *model);

			double getFutureTransitionOwn();

			double movingProbability(Household* household, HM_Model *model);

		private:

			bool futureTransitionOwn = false;
		};
	}
}
