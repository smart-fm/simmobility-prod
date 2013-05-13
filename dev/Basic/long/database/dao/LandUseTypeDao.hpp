/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LandUseTypeDao.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on May 7, 2013, 3:59 PM
 */
#pragma once

#include "database/dao/AbstractDao.hpp"
#include "database/entity/LandUseType.hpp"


namespace sim_mob {
    namespace long_term {

        DAO_DECLARE_CALLBACKS(LandUseType);

        /**
         * Data Access Object to LandUseType table on datasource.
         */
        class LandUseTypeDao : public AbstractDao<LandUseType> {
        public:
            LandUseTypeDao(DBConnection* connection);
            virtual ~LandUseTypeDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row. 
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void FromRow(Row& result, LandUseType& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void ToRow(LandUseType& data, Parameters& outParams, bool update);
        };
    }
}

