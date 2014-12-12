/*
 * AwakeningDao.hpp
 *
 *  Created on: 24 Nov, 2014
 *      Author: Chetan Rogbeer <chetan.rogbeer>
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/Awakening.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class AwakeningDao : public db::SqlAbstractDao<Awakening>
		{
	        public:
	            AwakeningDao(db::DB_Connection& connection);
	            virtual ~AwakeningDao();

	        private:
	            /**
	             * Fills the given outObj with all values contained on Row.
	             * @param result row with data to fill the out object.
	             * @param outObj to fill.
	             */
	            void fromRow(db::Row& result, Awakening& outObj);

	            /**
	             * Fills the outParam with all values to insert or update on datasource.
	             * @param data to get values.
	             * @param outParams to put the data parameters.
	             * @param update tells if operation is an Update or Insert.
	             */
	            void toRow(Awakening& data, db::Parameters& outParams, bool update);
		};
	}
}
