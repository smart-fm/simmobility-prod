/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   IndividualDao.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on May 7, 2013, 3:59 PM
 */
#pragma once

#include "database/dao/AbstractDao.hpp"
#include "database/entity/Individual.hpp"


namespace sim_mob {
    namespace long_term {

        DAO_DECLARE_CALLBACKS(Individual);
        /**
         * Data Access Object to Individual table on datasource.
         */
        class IndividualDao : public AbstractDao<Individual> {
        public:
            IndividualDao(DBConnection* connection);
            virtual ~IndividualDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row. 
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void FromRow(Row& result, Individual& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void ToRow(Individual& data, Parameters& outParams, bool update);
        };
    }
}

