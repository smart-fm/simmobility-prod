//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * mtz.hpp
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
		class Mtz
		{
		public:
			Mtz(BigSerial id = 0, BigSerial planningSubzoneId = 0, std::string name = "" );
			virtual ~Mtz();

			Mtz(Mtz & source);
			Mtz& operator=(Mtz& source);

			BigSerial getId() const;
			BigSerial getPlanningSubzoneId() const;
			std::string getName() const;

			friend std::ostream& operator<<(std::ostream& strm, const Mtz& data);

		private:
			BigSerial id;
			BigSerial planningSubzoneId;
			std::string name;
		};
	}
} /* namespace sim_mob */

