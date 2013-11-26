//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PredayDao.hpp
 *
 *  Created on: Nov 15, 2013
 *      Author: Harish Loganathan
 */

#pragma once

#include "database/dao/AbstractDao.hpp"
#include "database/DB_Connection.hpp"
#include "behavioral/params/PersonParams.hpp"

namespace sim_mob {
namespace medium {
/**
 * Data access object for Population tables
 */
class PopulationDao : public db::AbstractDao<PersonParams> {
public:
	PopulationDao(db::DB_Connection& connection);
	virtual ~PopulationDao();

private:
    /**
     * Virtual override.
     * Fills the given outObj with all values contained on Row.
     * @param result row with data to fill the out object.
     * @param outObj to fill.
     */
    void fromRow(db::Row& result, PersonParams& outObj);

    /**
     * Virtual override.
     * Fills the outParam with all values to insert or update on datasource.
     * @param data to get values.
     * @param outParams to put the data parameters.
     * @param update tells if operation is an Update or Insert.
     */
    void toRow(PersonParams& data, db::Parameters& outParams, bool update);


};
} // end namespace medium
} // end namespace sim_mib
