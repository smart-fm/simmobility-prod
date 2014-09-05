//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


/*
 * ResidentialStatus.hpp
 *
 *  Created on: 4 Sep, 2014
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class ResidentialStatus
		{
		public:
			ResidentialStatus(BigSerial id = INVALID_ID, std::string name = EMPTY_STR);
			virtual ~ResidentialStatus();

			BigSerial getId() const;
			std::string getName() const;

			friend std::ostream& operator<<(std::ostream& strm, const ResidentialStatus& data);

		private:

			friend class ResidentialStatusDao;

			BigSerial id;
			std::string name;
		};
	}
}


