/*
 * StrategicPlan.hpp
 *
 *  Created on: Jun 6, 2012
 *      Author: vuvinhan
 */

#ifndef STRATEGICPLAN_HPP_
#define STRATEGICPLAN_HPP_

#include "Plan.hpp"

namespace sim_mob
{
	class StrategicPlan : public sim_mob::Plan {
	public:
		StrategicPlan();
		virtual ~StrategicPlan();
	};
}

#endif /* STRATEGICPLAN_HPP_ */
