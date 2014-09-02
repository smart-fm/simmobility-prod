//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   LandUseZoneDao.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Mar 21, 2014, 5:17 PM
 */
#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/LandUseZone.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {

        /**
         * Data Access Object to LandUseZone table on datasource.
         */
        class LandUseZoneDao : public db::SqlAbstractDao<LandUseZone> {
        public:
            LandUseZoneDao(db::DB_Connection& connection);
            virtual ~LandUseZoneDao();

        private:

            /**
             * Fills the given outObj with all values contained on Row. 
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, LandUseZone& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(LandUseZone& data, db::Parameters& outParams, bool update);
        };
    }
}