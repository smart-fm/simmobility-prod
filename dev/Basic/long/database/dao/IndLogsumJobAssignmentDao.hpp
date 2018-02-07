/*
 * IndLogsumJobAssignmentDao.hpp
 *
 *  Created on: 28 Aug 2017
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/IndLogsumJobAssignment.hpp"


namespace sim_mob {
    namespace long_term {

        /**
         * Data Access Object to logsum table on datasource.
         */
        class IndLogsumJobAssignmentDao : public db::SqlAbstractDao<IndLogsumJobAssignment> {
        public:
        	IndLogsumJobAssignmentDao(db::DB_Connection& connection);
            virtual ~IndLogsumJobAssignmentDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, IndLogsumJobAssignment& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(IndLogsumJobAssignment& data, db::Parameters& outParams, bool update);

        public:
            std::vector<IndLogsumJobAssignment*> loadLogsumByIndividualId(BigSerial individualId);
        };
    }
}
