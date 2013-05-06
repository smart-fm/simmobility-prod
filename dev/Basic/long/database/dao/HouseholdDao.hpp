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
            void ToRow(Household& data, Parameters& outParams, bool update);
        };
    }
}



