/*
 * UnitSaleDao.hpp
 *
 *  Created on: Dec 15, 2015
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/UnitSale.hpp"


namespace sim_mob {
    namespace long_term {

        /**
         * Data Access Object to unit_sale table on datasource.
         */
        class UnitSaleDao : public db::SqlAbstractDao<UnitSale> {
        public:
        	UnitSaleDao(db::DB_Connection& connection);
            virtual ~UnitSaleDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, UnitSale& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(UnitSale& data, db::Parameters& outParams, bool update);

        public:
            void insertUnitSale(UnitSale& bid,std::string schema);
        };
    }
}


