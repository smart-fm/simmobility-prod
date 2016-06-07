//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * AccessibilityFixedPzid.hpp
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
		class AccessibilityFixedPzid
		{
		public:
			AccessibilityFixedPzid(int _id=0, int _planningAreaId = 0, std::string _dgp="", double acc_t_mfg = .0, double acc_t_off = .0);

			virtual ~AccessibilityFixedPzid();

			int getId() const;
			int getPlanningAreaId() const;
			std::string getDgp() const;
			double getAccTMfg() const;
			double getAccTOff() const;

            /**
             * Operator to print the data.
             */
            friend std::ostream& operator<<(std::ostream& strm, const AccessibilityFixedPzid& data);

        private:

            friend class AccessibilityFixedPzidDao;

            int id;
			int planningAreaId;
			std::string dgp;
			double accTMfg;
			double accTOff;
		};
	}
}
