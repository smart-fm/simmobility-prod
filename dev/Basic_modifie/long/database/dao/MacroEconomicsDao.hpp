/*
 * MacroEconomicsDao.hpp
 *
 *  Created on: Jan 14, 2015
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/MacroEconomics.hpp"


namespace sim_mob {
    namespace long_term {

        /**
         * Data Access Object to MacroEconomics table on datasource.
         */
        class MacroEconomicsDao : public db::SqlAbstractDao<MacroEconomics> {
        public:
        	MacroEconomicsDao(db::DB_Connection& connection);
            virtual ~MacroEconomicsDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, MacroEconomics& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(MacroEconomics& data, db::Parameters& outParams, bool update);
        };
    }
}

