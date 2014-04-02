//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   DeveloperDao.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Mar 5, 2015, 5:17 PM
 */
#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/Developer.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {
        /**
         * Data Access Object to Developer table on datasource.
         */
        class DeveloperDao : public db::SqlAbstractDao<Developer> {
        public:
            DeveloperDao(db::DB_Connection& connection);
            virtual ~DeveloperDao();

        private:
            
            /**
             * Fills the given outObj with all values contained on Row. 
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, Developer& outObj);
            
            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(Developer& data, db::Parameters& outParams, bool update);
        };
    }
}



