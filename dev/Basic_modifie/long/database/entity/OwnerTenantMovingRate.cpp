//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * ownerTenantMovingRate.cpp
 *
 *  Created on: 5 Feb 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/OwnerTenantMovingRate.hpp>

namespace sim_mob
{
	namespace long_term
	{
		OwnerTenantMovingRate::OwnerTenantMovingRate( BigSerial _id, int _ageCategory, int _ownerPopulation, int _tenantPopulation, int _ownerMovingPercentage,int _tenantMovingPercentage, bool _day_zero)
		{
			id = _id;
			ageCategory = _ageCategory;
			ownerPopulation = _ownerPopulation;
			tenantPopulation = _tenantPopulation;
			ownerMovingPercentage = _ownerMovingPercentage;
			tenantMovingPercentage = _tenantMovingPercentage;
			day_zero = _day_zero;
		}

		OwnerTenantMovingRate::~OwnerTenantMovingRate() {}

		BigSerial OwnerTenantMovingRate::getId() const
		{
			return id;
		}

		int OwnerTenantMovingRate::getAgeCategory() const
		{
			return ageCategory;
		}

		double OwnerTenantMovingRate::getOwnerPopulation() const
		{
			return ownerPopulation;
		}

		double OwnerTenantMovingRate::getTenantPopulation() const
		{
			return tenantPopulation;
		}

		double OwnerTenantMovingRate::getOwnerMovingPercentage() const
		{
			return ownerMovingPercentage;
		}

		double OwnerTenantMovingRate::getTenantMovingPercentage() const
		{
			return tenantMovingPercentage;
		}

		bool OwnerTenantMovingRate::getDayZero() const
		{
			return day_zero;
		}

		std::ostream& operator<<(std::ostream& strm, const OwnerTenantMovingRate& data)
		{
			return strm << "{"
					<< "\"id\":\"" << data.id << "\","
					<< "\"ageCategory\":\"" << data.ageCategory << "\","
					<< "\"\"ownerPopulation:\"" << data.ownerPopulation << "\","
					<< "\"\"tenantPopulation:\"" << data.tenantPopulation << "\","
					<< "\"\"ownerMovingPercentage:\"" << data.ownerMovingPercentage << "\","
					<< "\"\"tenantMovingPercentage:\"" << data.tenantMovingPercentage << "\","
					<< "\"\"day zero:\"" << data.day_zero << "\","
					<< "}";
		}

	}
} /* namespace sim_mob */
