/*
 * TotalBuildingSpaceDao.hpp
 *
 *  Created on: Dec 5, 2014
 *      Author: gishara
 */
#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/TotalBuildingSpace.hpp"


namespace sim_mob
{
    namespace long_term
    {
        /**
         * Data Access Object to get the data to calculate GPR values.
         */
        class TotalBuildingSpaceDao : public db::SqlAbstractDao<TotalBuildingSpace>
        {
        public:
        	TotalBuildingSpaceDao(db::DB_Connection& connection);
            virtual ~TotalBuildingSpaceDao();

        private:
            /**
             * Fills the given outObj with all values contained on Row.
             * @param result row with data to fill the out object.
             * @param outObj to fill.
             */
            void fromRow(db::Row& result, TotalBuildingSpace& outObj);

            /**
             * Fills the outParam with all values to insert or update on datasource.
             * @param data to get values.
             * @param outParams to put the data parameters.
             * @param update tells if operation is an Update or Insert.
             */
            void toRow(TotalBuildingSpace& data, db::Parameters& outParams, bool update);

        public:
            /*
            * Get the parcels with no buildings as a vector
            */
            std::vector<TotalBuildingSpace*> getBuildingSpaces();
        };
    }
}
