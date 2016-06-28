//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)


/*
 * ROILimitsDao.hpp
 *
 *  Created on: 17 May 2016
 *      Author: gishara
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/ROILimits.hpp"

namespace sim_mob
{
    namespace long_term
    {
        /**
         * Data Access Object to roi_limits table on data source.
         */
        class ROILimitsDao : public db::SqlAbstractDao<ROILimits>
        {
        public:
        	ROILimitsDao(db::DB_Connection& connection);
            virtual ~ROILimitsDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, ROILimits& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(ROILimits& data, db::Parameters& outParams, bool update);
        };
    }
}

