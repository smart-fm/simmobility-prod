/*
 * JobDao.hpp
 *
 *  Created on: 23 Apr, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/Job.hpp"

namespace sim_mob
{
	namespace long_term
	{
		 /**
		 * Data Access Object to Household table on datasource.
		 */
		class JobDao : public db::SqlAbstractDao<Job>
		{
		public:
			JobDao(db::DB_Connection& connection);
			virtual ~JobDao();

		private:

			/**
			 * Fills the given outObj with all values contained on Row.
			 * @param result row with data to fill the out object.
			 * @param outObj to fill.
			 */
			void fromRow(db::Row& result, Job& outObj);

			/**
			 * Fills the outParam with all values to insert or update on datasource.
			 * @param data to get values.
			 * @param outParams to put the data parameters.
			 * @param update tells if operation is an Update or Insert.
			 */
			void toRow(Job& data, db::Parameters& outParams, bool update);
		};
	}

}
