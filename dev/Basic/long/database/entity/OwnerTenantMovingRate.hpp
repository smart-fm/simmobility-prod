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
								   int _tenantMovingPercentage = 0);

			virtual ~OwnerTenantMovingRate();

			BigSerial getId() const;
			int getAgeCategory() const;
			int getOwnerPopulation() const;
			int getTenantPopulation() const;
			int getOwnerMovingPercentage() const;
			int getTenantMovingPercentage() const;

		    /**
		    * Operator to print the data.
		    */
		    friend std::ostream& operator<<(std::ostream& strm, const OwnerTenantMovingRate& data);

		private:
		    friend class OwnerTenantMovingRateDao;

			BigSerial id = 0;
			int ageCategory = 0;
			int ownerPopulation = 0;
			int tenantPopulation = 0;
			int ownerMovingPercentage = 0;
			int tenantMovingPercentage = 0;

		};

	}
}
