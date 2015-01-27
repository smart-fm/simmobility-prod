/*
 * unitTypeDao.hpp
 *
 *  Created on: Nov 24, 2014
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/UnitType.hpp"


namespace sim_mob {
    namespace long_term {

        /**
         * Data Access Object to unit_type table on datasource.
         */
        class UnitTypeDao : public db::SqlAbstractDao<UnitType> {
        public:
            UnitTypeDao(db::DB_Connection& connection);
            virtual ~UnitTypeDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, UnitType& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(UnitType& data, db::Parameters& outParams, bool update);
        };
    }
}
