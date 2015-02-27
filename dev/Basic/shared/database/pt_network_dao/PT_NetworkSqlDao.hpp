//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/**
 * Pt_NetworkSqlDao.hpp
 *
 *  Created on: Feb 24, 2015
 *  \author: Prabhuraj
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/DB_Connection.hpp"
#include "entities/params/Pt_network_entities.hpp"

namespace sim_mob{

class PT_VerticesSqlDao : public db::SqlAbstractDao<PT_NetworkVertices> {
public:
	PT_VerticesSqlDao(db::DB_Connection& connection);
	virtual ~PT_VerticesSqlDao();

private:
    /**
     * Virtual override.
     * Fills the given outObj with all values contained on Row.
     * @param result row with data to fill the out object.
     * @param outObj to fill.
     */
    void fromRow(db::Row& result, PT_NetworkVertices& outObj);

    /**
     * Virtual override.
     * Fills the outParam with all values to insert or update on datasource.
     * @param data to get values.
     * @param outParams to put the data parameters.
     * @param update tells if operation is an Update or Insert.
     */
    void toRow(PT_NetworkVertices& data, db::Parameters& outParams, bool update);
};


class Pt_EdgesSqlDao : public db::SqlAbstractDao<PT_NetworkEdges> {
public:
	Pt_EdgesSqlDao(db::DB_Connection& connection);
	virtual ~Pt_EdgesSqlDao();

private:
    /**
     * Virtual override.
     * Fills the given outObj with all values contained on Row.
     * @param result row with data to fill the out object.
     * @param outObj to fill.
     */
    void fromRow(db::Row& result, PT_NetworkEdges& outObj);

    /**
     * Virtual override.
     * Fills the outParam with all values to insert or update on datasource.
     * @param data to get values.
     * @param outParams to put the data parameters.
     * @param update tells if operation is an Update or Insert.
     */
    void toRow(PT_NetworkEdges& data, db::Parameters& outParams, bool update);
};
}// End sim::mob namespace
