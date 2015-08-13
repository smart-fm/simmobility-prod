//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)


/*
 * PopulationPerPlanningAreaDao.hpp
 *
 *  Created on: 13 Aug, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */


#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/entity/PopulationPerPlanningArea.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class PopulationPerPlanningAreaDao : public db::SqlAbstractDao<PopulationPerPlanningArea>
		{
	        public:
			PopulationPerPlanningAreaDao(db::DB_Connection& connection);
	            virtual ~PopulationPerPlanningAreaDao();

	        private:
	            /**
	             * Fills the given outObj with all values contained on Row.
	             * @param result row with data to fill the out object.
	             * @param outObj to fill.
	             */
	            void fromRow(db::Row& result, PopulationPerPlanningArea& outObj);

	            /**
	             * Fills the outParam with all values to insert or update on datasource.
	             * @param data to get values.
	             * @param outParams to put the data parameters.
	             * @param update tells if operation is an Update or Insert.
	             */
	            void toRow(PopulationPerPlanningArea& data, db::Parameters& outParams, bool update);
		};
	}
}

