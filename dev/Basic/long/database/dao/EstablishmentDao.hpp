/*
 * EstablishmentDao.h
 *
 *  Created on: 23 Apr, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/Establishment.hpp"

namespace sim_mob
{
	namespace long_term
	{
		/**
		 * Data Access Object to Household table on datasource.
		 */
		class EstablishmentDao : public db::SqlAbstractDao<Establishment>
		{
		public:
			EstablishmentDao(db::DB_Connection& connection);
			virtual ~EstablishmentDao();

	    private:

	            /**
	             * Fills the given outObj with all values contained on Row.
	             * @param result row with data to fill the out object.
	             * @param outObj to fill.
	             */
	            void fromRow(db::Row& result, Establishment& outObj);

	            /**
	             * Fills the outParam with all values to insert or update on datasource.
	             * @param data to get values.
	             * @param outParams to put the data parameters.
	             * @param update tells if operation is an Update or Insert.
	             */
	            void toRow(Establishment& data, db::Parameters& outParams, bool update);
		};
	}
}


