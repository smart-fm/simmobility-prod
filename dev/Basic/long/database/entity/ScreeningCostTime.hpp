//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * screeningCostTime.hpp
 *
 *  Created on: 20 Jan 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include <string>
#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class ScreeningCostTime
		{
		public:
			ScreeningCostTime(  int id = 0, int planningAreaOrigin = 0, std::string areaOrigin = "", int planningAreaDestination = 0, std::string areaDestination = "", double time = .0,
								double cost = .0 );

			virtual ~ScreeningCostTime();

			int getId() const;
			int getPlanningAreaOrigin() const;
			std::string getAreaOrigin() const;
			int getPlanningAreaDestination() const;
			std::string getAreaDestination() const;
			double getTime() const;
			double getCost() const;

            /**
             * Operator to print the data.
             */
            friend std::ostream& operator<<(std::ostream& strm, const ScreeningCostTime& data);

        private:
            friend class ScreeningCostTimeDao;

            int id;
			int planningAreaOrigin;
			std::string areaOrigin;
			int planningAreaDestination;
			std::string areaDestination;
			double time;
			double cost;
		};
	}
}
