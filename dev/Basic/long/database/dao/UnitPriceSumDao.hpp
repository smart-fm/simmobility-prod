/*
 * UnitPriceDao.hpp
 *
 *  Created on: Aug 20, 2015
 *      Author: gishara
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/UnitPriceSum.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {

        /**
         * Data Access Object to get UnitPriceSum from datasource.
         */
        class UnitPriceSumDao : public db::SqlAbstractDao<UnitPriceSum> {
        public:
        	UnitPriceSumDao(db::DB_Connection& connection);
            virtual ~UnitPriceSumDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, UnitPriceSum& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(UnitPriceSum& data, db::Parameters& outParams, bool update);
        };
    }
}

