//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * Pt_NetworkSqlDao.hpp
 *
 *  Created on: Feb 24, 2015
 *      Author: Prabhuraj
 */

#pragma once

#include "database/dao/SqlAbstractDao.hpp"
#include "database/DB_Connection.hpp"
#include "entities/params/PersonParams.hpp"


class Pt_VerticesSqlDao : public db::SqlAbstractDao<Pt_network_vertices*> {
public:
	Pt_VerticesSqlDao(db::DB_Connection& connection);
	virtual ~Pt_VerticesSqlDao();

private:
    /**
     * Virtual override.
     * Fills the given outObj with all values contained on Row.
     * @param result row with data to fill the out object.
     * @param outObj to fill.
     */
    void fromRow(db::Row& result, Pt_network_vertices& outObj);

    /**
     * Virtual override.
     * Fills the outParam with all values to insert or update on datasource.
     * @param data to get values.
     * @param outParams to put the data parameters.
     * @param update tells if operation is an Update or Insert.
     */
    void toRow(Pt_network_vertices& data, db::Parameters& outParams, bool update);
};



class Pt_EdgesSqlDao : public db::SqlAbstractDao<pt_network_edges*> {
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
    void fromRow(db::Row& result, pt_network_edges& outObj);

    /**
     * Virtual override.
     * Fills the outParam with all values to insert or update on datasource.
     * @param data to get values.
     * @param outParams to put the data parameters.
     * @param update tells if operation is an Update or Insert.
     */
    void toRow(pt_network_edges& data, db::Parameters& outParams, bool update);
};
