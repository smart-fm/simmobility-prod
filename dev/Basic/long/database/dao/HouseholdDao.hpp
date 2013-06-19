/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HouseholdDao.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 23, 2013, 5:17 PM
 */
#pragma once
#include "database/dao/AbstractDao.hpp"
#include "database/entity/Household.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {

        DAO_DECLARE_CALLBACKS(Household);
        /**
         * Data Access Object to Household table on datasource.
         */
        class HouseholdDao : public AbstractDao<Household> {
        public:
            HouseholdDao(DBConnection* connection);
            virtual ~HouseholdDao();

        private:
            
            /**
             * Fills the given outObj with all values contained on Row. 
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void FromRow(Row& result, Household& outObj);
            
            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void ToRow(Household& data, Parameters& outParams, bool update);
        };
    }
}



