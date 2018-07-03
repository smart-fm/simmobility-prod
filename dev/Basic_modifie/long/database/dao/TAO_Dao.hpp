/*
 * TAO_Dao.hpp
 *
 *  Created on: Jul 30, 2015
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/TAO.hpp"


namespace sim_mob {
    namespace long_term {
        /**
         * Data Access Object to TAO table on data source.
         */
        class TAO_Dao : public db::SqlAbstractDao<TAO> {
        public:
        	TAO_Dao(db::DB_Connection& connection);
            virtual ~TAO_Dao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, TAO& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(TAO& data, db::Parameters& outParams, bool update);
        };
    }
}
