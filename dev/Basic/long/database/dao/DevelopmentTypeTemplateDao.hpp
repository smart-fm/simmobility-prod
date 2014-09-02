//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   DevelopmentTypeTemplateDao.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Mar 25, 2014, 5:17 PM
 */
#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/DevelopmentTypeTemplate.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {
        /**
         * Data Access Object to DevelopmentTypeTemplate table on datasource.
         */
        class DevelopmentTypeTemplateDao : public db::SqlAbstractDao<DevelopmentTypeTemplate> {
        public:
            DevelopmentTypeTemplateDao(db::DB_Connection& connection);
            virtual ~DevelopmentTypeTemplateDao();

        private:
            
            /**
             * Fills the given outObj with all values contained on Row. 
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, DevelopmentTypeTemplate& outObj);
            
            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(DevelopmentTypeTemplate& data, db::Parameters& outParams, bool update);
        };
    }
}



