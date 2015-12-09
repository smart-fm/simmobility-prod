/*
 * TimeCheck.hpp
 *
 *  Created on: 9 Dec 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once
#include <ctime>

namespace sim_mob
{
    namespace long_term
    {
		class TimeCheck
		{
		public:
			TimeCheck();
			virtual ~TimeCheck();

			double getClockTime();
			double getProcessTime();

			time_t  start_clock;
			clock_t start_process;
		};
    }
}



