/*
 * Plan.hpp
 *
 *  Created on: Jun 6, 2012
 *      Author: vuvinhan
 */

#ifndef PLAN_HPP_
#define PLAN_HPP_

namespace sim_mob
{
	class Plan {
	public:
		Plan();
		virtual ~Plan();
		Plan* getPlan();
	};
}
#endif /* PLAN_HPP_ */
