//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * planningArea.hpp
 *
 *  Created on: 30 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class PlanningArea
		{
		public:
			PlanningArea(BigSerial id = 0, std::string name  = "" );
			virtual ~PlanningArea();

			PlanningArea(const PlanningArea& source);
			PlanningArea& operator=(const PlanningArea& source);

			BigSerial getId() const;
			std::string getName() const;

			friend std::ostream& operator<<(std::ostream& strm, const PlanningArea& data);

		private:

			BigSerial id;
			std::string name;
		};
	}

}

