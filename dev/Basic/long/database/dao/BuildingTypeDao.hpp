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
#include "database/entity/BuildingType.hpp"


namespace sim_mob {
    namespace long_term {

        DAO_DECLARE_CALLBACKS(BuildingType);
        /**
         * Data Access Object to BuildingType table on datasource.
         */
        class BuildingTypeDao : public AbstractDao<BuildingType> {
        public:
            BuildingTypeDao(DBConnection* connection);
            virtual ~BuildingTypeDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row. 
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void FromRow(Row& result, BuildingType& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void ToRow(BuildingType& data, Parameters& outParams, bool update);
        };
    }
}

