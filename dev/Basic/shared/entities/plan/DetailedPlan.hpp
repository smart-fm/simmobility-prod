/*
 * DetailedPlan.hpp
 *
 *  Created on: Jun 6, 2012
 *      Author: vuvinhan
 */

#ifndef DETAILEDPLAN_HPP_
#define DETAILEDPLAN_HPP_

#include "Plan.hpp"

namespace sim_mob
{
	class DetailedPlan : public sim_mob::Plan  {
	public:
		DetailedPlan();
		virtual ~DetailedPlan();
	};
}

#endif /* DETAILEDPLAN_HPP_ */
