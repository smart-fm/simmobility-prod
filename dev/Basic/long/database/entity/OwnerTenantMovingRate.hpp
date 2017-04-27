//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * ownerTenantMovingRate.hpp
 *
 *  Created on: 5 Feb 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smat.mit.edu>
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class OwnerTenantMovingRate
		{
		public:
			OwnerTenantMovingRate( BigSerial _id = 0, int _ageCategory = 0, int _ownerPopulation = 0, int _tenantPopulation = 0, int _ownerMovingPercentage = 0,
								   int _tenantMovingPercentage = 0,bool _day_zero = false);

			virtual ~OwnerTenantMovingRate();

			BigSerial getId() const;
			int getAgeCategory() const;
			double getOwnerPopulation() const;
			double getTenantPopulation() const;
			double getOwnerMovingPercentage() const;
			double getTenantMovingPercentage() const;
			bool getDayZero() const;

		    /**
		    * Operator to print the data.
		    */
		    friend std::ostream& operator<<(std::ostream& strm, const OwnerTenantMovingRate& data);

		private:
		    friend class OwnerTenantMovingRateDao;

			BigSerial id = 0;
			int ageCategory = 0;
			double ownerPopulation = 0;
			double tenantPopulation = 0;
			double ownerMovingPercentage = 0;
			double tenantMovingPercentage = 0;
			int day_zero = false;

		};

	}
}
