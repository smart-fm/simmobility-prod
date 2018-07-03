//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * TenureTransitionRate.hpp
 *
 *  Created on: 5 Feb 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

using namespace std;

namespace sim_mob {

	namespace long_term
	{
		class TenureTransitionRate
		{
		public:
			TenureTransitionRate( BigSerial id = 0, string ageGroup = "", string currentStatus = "", string futureStatus = "", double rate = 0.0 );
			virtual ~TenureTransitionRate();

			BigSerial getId() const;
			string getAgeGroup() const;
			string getCurrentStatus() const;
			string getFutureStatus() const;
			double getRate() const;

			 /**
			 * Operator to print the data.
			 */
			friend std::ostream& operator<<(std::ostream& strm, const TenureTransitionRate& data);

		private:

			friend class TenureTransitionRateDao;

			BigSerial id;
			string ageGroup;
			string currentStatus;
			string futureStatus;
			double rate;

		};
	}
}
