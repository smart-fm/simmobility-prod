//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * OwnerTenantMovingRateDao.hpp
 *
 *  Created on: 5 Feb 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/OwnerTenantMovingRate.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class OwnerTenantMovingRateDao : public db::SqlAbstractDao<OwnerTenantMovingRate>
		{
	        public:
			OwnerTenantMovingRateDao(db::DB_Connection& connection);
	        virtual ~OwnerTenantMovingRateDao();

	        private:
	            /**
	             * Fills the given outObj with all values contained on Row.
	             * @param result row with data to fill the out object.
	             * @param outObj to fill.
	             */
	            void fromRow(db::Row& result, OwnerTenantMovingRate& outObj);

	            /**
	             * Fills the outParam with all values to insert or update on datasource.
	             * @param data to get values.
	             * @param outParams to put the data parameters.
	             * @param update tells if operation is an Update or Insert.
	             */
	            void toRow(OwnerTenantMovingRate& data, db::Parameters& outParams, bool update);
		};
	}
}
