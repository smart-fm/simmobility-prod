/*
 * SlaParcelDao.hpp
 *
 *  Created on: Aug 27, 2014
 *      Author: gishara
 */
#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/SlaParcel.hpp"


namespace sim_mob {
    namespace long_term {
        /**
         * Data Access Object to SlaParcel table on data source.
         */
        class SlaParcelDao : public db::SqlAbstractDao<SlaParcel> {
        public:
        	SlaParcelDao(db::DB_Connection& connection);
            virtual ~SlaParcelDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, SlaParcel& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(SlaParcel& data, db::Parameters& outParams, bool update);
        };
    }
}
