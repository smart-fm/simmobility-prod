//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)


/*
 * LogsumMtzV2Dao.hpp
 *
 *  Created on: 27 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/LogsumMtzV2.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class LogsumMtzV2Dao : public db::SqlAbstractDao<LogsumMtzV2>
		{
	        public:
			LogsumMtzV2Dao(db::DB_Connection& connection);
	            virtual ~LogsumMtzV2Dao();

	        private:
	            /**
	             * Fills the given outObj with all values contained on Row.
	             * @param result row with data to fill the out object.
	             * @param outObj to fill.
	             */
	            void fromRow(db::Row& result, LogsumMtzV2& outObj);

	            /**
	             * Fills the outParam with all values to insert or update on datasource.
	             * @param data to get values.
	             * @param outParams to put the data parameters.
	             * @param update tells if operation is an Update or Insert.
	             */
	            void toRow(LogsumMtzV2& data, db::Parameters& outParams, bool update);
		};
	}
}

