//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)


/*
 * planningsubzone.hpp
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
		class PlanningSubzone
		{
		public:
			PlanningSubzone(BigSerial id = 0, BigSerial planningAreaId = 0, std::string name = "");
			virtual ~PlanningSubzone();

			PlanningSubzone(const PlanningSubzone &source);
			PlanningSubzone& operator=(const PlanningSubzone& source);

			BigSerial getId() const;
			BigSerial getPlanningAreaId() const;
			std::string getName() const;

			friend std::ostream& operator<<(std::ostream& strm, const PlanningSubzone& data);

		private:

			friend class PlanningSubzoneDao;

			BigSerial id;
			BigSerial planningAreaId;
			std::string name;
		};
	}
} /* namespace sim_mob */


