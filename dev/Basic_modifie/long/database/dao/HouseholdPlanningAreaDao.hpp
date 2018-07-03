/*
 * HouseholdPlanningAreaDao.hpp
 *
 *  Created on: 8 Mar 2016
 *      Author: gishara
 */

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/HouseholdPlanningArea.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {
        /**
         * Data Access Object to HouseholdPlanningArea view on datasource.
         */
        class HouseholdPlanningAreaDao : public db::SqlAbstractDao<HouseholdPlanningArea> {
        public:
        	HouseholdPlanningAreaDao(db::DB_Connection& connection);
            virtual ~HouseholdPlanningAreaDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, HouseholdPlanningArea& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(HouseholdPlanningArea& data, db::Parameters& outParams, bool update);
        };
    }
}
