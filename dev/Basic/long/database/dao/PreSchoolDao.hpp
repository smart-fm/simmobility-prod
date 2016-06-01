/*
 * PreSchoolDao.hpp
 *
 *  Created on: 15 Mar 2016
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/PreSchool.hpp"

namespace sim_mob {
    namespace long_term {
        /**
         * Data Access Object to pre_school table on data source.
         */
        class PreSchoolDao : public db::SqlAbstractDao<PreSchool> {
        public:
        	PreSchoolDao(db::DB_Connection& connection);
            virtual ~PreSchoolDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, PreSchool& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(PreSchool& data, db::Parameters& outParams, bool update);
        };
    }
}
