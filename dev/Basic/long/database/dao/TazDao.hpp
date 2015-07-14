//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   TemplateDao.cpp
 * Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 *
 * Created on June 23, 2015, 1:12 PM
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/Taz.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {
        /**
         * Data Access Object to Taz table on datasource.
         */
        class TazDao : public db::SqlAbstractDao<Taz> {
        public:
            TazDao(db::DB_Connection& connection);
            virtual ~TazDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, Taz& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(Taz& data, db::Parameters& outParams, bool update);
        };
    }
}
