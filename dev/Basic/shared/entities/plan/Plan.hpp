//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * Plan.hpp
 *
 *  Created on: Jun 6, 2012
 *      Author: vuvinhan
 */

#pragma once

namespace sim_mob
{
	class Plan {
	public:
		Plan();
		virtual ~Plan();
		Plan* getPlan();
	};
}
