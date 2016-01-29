//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * WillingnessToPaySubModelcpp.hpp
 *
 *  Created on: 29 Jan 2016
 *  Author: Chetan Rogbeer <chetan.rogbeeer@smart.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"
#include "database/entity/Unit.hpp"
#include "database/entity/Household.hpp"
#include "model/HM_Model.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class WillingnessToPaySubModel
		{
		public:
			WillingnessToPaySubModel();
			virtual ~WillingnessToPaySubModel();

			double CalculateWillingnessToPay(const Unit* unit, const Household* household, double& wtp_e, double day, HM_Model *model);
		};
	}
}
