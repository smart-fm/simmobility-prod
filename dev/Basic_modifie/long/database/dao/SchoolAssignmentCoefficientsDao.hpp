/*
 * SchoolAssignmentCoefficientsDao.hpp
 *
 *  Created on: 9 Mar 2016
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/SchoolAssignmentCoefficients.hpp"


namespace sim_mob
{
    namespace long_term
    {
        /**
         * Data Access Object to school_assignment_coefficients table on datasource.
         */
        class SchoolAssignmentCoefficientsDao : public db::SqlAbstractDao<SchoolAssignmentCoefficients>
        {
        public:
        	SchoolAssignmentCoefficientsDao(db::DB_Connection& connection);
            virtual ~SchoolAssignmentCoefficientsDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, SchoolAssignmentCoefficients& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(SchoolAssignmentCoefficients& data, db::Parameters& outParams, bool update);

        };
    }
}
