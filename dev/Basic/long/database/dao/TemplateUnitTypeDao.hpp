//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   TemplateUnitTypeDao.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Mar 25, 2014, 5:17 PM
 */
#pragma once
#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/TemplateUnitType.hpp"

using namespace boost;
namespace sim_mob {

    namespace long_term {
        /**
         * Data Access Object to DevelopmentTypeTemplate table on datasource.
         */
        class TemplateUnitTypeDao : public db::SqlAbstractDao<TemplateUnitType> {
        public:
            TemplateUnitTypeDao(db::DB_Connection& connection);
            virtual ~TemplateUnitTypeDao();

        private:
            
            /**
             * Fills the given outObj with all values contained on Row. 
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, TemplateUnitType& outObj);
            
            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(TemplateUnitType& data, db::Parameters& outParams, bool update);
        };
    }
}



