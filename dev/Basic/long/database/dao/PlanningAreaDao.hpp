//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * PlanningAreaDao.hpp
 *
 * Created on: July 30, 2015
 * Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */
#pragma once

#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/PlanningArea.hpp"

namespace sim_mob
{
    namespace long_term
    {
        /**
         * Data Access Object to getParcelsWithHDB function on data source.
         */
        class PlanningAreaDao : public db::SqlAbstractDao<PlanningArea>
        {
        public:
        	PlanningAreaDao(db::DB_Connection& connection);
            virtual ~PlanningAreaDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, PlanningArea& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(PlanningArea& data, db::Parameters& outParams, bool update);
        };
    }
}
