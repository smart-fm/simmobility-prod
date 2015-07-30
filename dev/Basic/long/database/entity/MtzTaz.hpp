//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * MtzTaz.hpp
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
		class MtzTaz
		{
		public:
			MtzTaz(BigSerial mtzId =0, BigSerial tazId = 0);
			virtual ~MtzTaz();

			MtzTaz(MtzTaz& source);
			MtzTaz& operator=(MtzTaz& source);

			BigSerial getMtzId() const;
			BigSerial getTazId() const;

			friend std::ostream& operator<<(std::ostream& strm, const MtzTaz& data);

		private:
			BigSerial mtzId;
			BigSerial tazId;
		};
	}
} /* namespace sim_mob */
