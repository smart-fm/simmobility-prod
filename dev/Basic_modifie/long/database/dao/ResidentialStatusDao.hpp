//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


/*
 * ResidentialStatusDao.hpp
 *
 *  Created on: 4 Sep, 2014
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/ResidentialStatus.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class ResidentialStatusDao : public db::SqlAbstractDao<ResidentialStatus>
		{
		public:
			ResidentialStatusDao(db::DB_Connection& connection);
			virtual ~ResidentialStatusDao();

		private:
			/**
			 * Fills the given outObj with all values contained on Row.
			 * @param result row with data to fill the out object.
			 * @param outObj to fill.
			 */
			void fromRow(db::Row& result, ResidentialStatus& outObj);

			/**
			 * Fills the outParam with all values to insert or update on datasource.
			 * @param data to get values.
			 * @param outParams to put the data parameters.
			 * @param update tells if operation is an Update or Insert.
			 */
			void toRow(ResidentialStatus& data, db::Parameters& outParams, bool update);
		};
	}
}
