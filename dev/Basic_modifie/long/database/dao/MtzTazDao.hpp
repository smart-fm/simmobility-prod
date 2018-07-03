//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licenced under of the terms of the MIT licence, as described in the file:
//licence.txt (www.opensource.org\licences\MIT)

/*
 * MtzTazDao.hpp
 *
 *  Created on: 31 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */


#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/MtzTaz.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class MtzTazDao : public db::SqlAbstractDao<MtzTaz>
		{
		public:
			MtzTazDao( db::DB_Connection& connection );
			virtual ~MtzTazDao();

		private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, MtzTaz& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(MtzTaz& data, db::Parameters& outParams, bool update);
		};
	}
}
