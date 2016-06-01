/*
 * PrimarySchoolDao.hpp
 *
 *  Created on: 10 Mar 2016
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/PrimarySchool.hpp"

namespace sim_mob {
    namespace long_term {
        /**
         * Data Access Object to primary_school table on data source.
         */
        class PrimarySchoolDao : public db::SqlAbstractDao<PrimarySchool> {
        public:
        	PrimarySchoolDao(db::DB_Connection& connection);
            virtual ~PrimarySchoolDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, PrimarySchool& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(PrimarySchool& data, db::Parameters& outParams, bool update);
        };
    }
}
