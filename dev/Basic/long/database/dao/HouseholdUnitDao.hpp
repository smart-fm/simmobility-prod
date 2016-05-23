/*
 * HouseholdUnitDao.hpp
 *
 *  Created on: 28 Mar 2016
 *      Author: gishara
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/HouseholdUnit.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {
        /**
         * Data Access Object to household_unit table on datasource.
         */
        class HouseholdUnitDao : public db::SqlAbstractDao<HouseholdUnit> {
        public:
        	HouseholdUnitDao(db::DB_Connection& connection);
            virtual ~HouseholdUnitDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, HouseholdUnit& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(HouseholdUnit& data, db::Parameters& outParams, bool update);

        public:
            void insertHouseholdUnit(HouseholdUnit& houseHoldUnit,std::string schema);
        };
    }
}
