//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/params/PT_NetworkEntities.hpp"
#include "database/dao/SqlAbstractDao.hpp"
#include "database/DB_Connection.hpp"

namespace sim_mob
{

/**
 * DAO for PT vertex table
 *
 * \author Prabhuraj
 */
class PT_VerticesSqlDao: public db::SqlAbstractDao<PT_NetworkVertex>
{
public:
	PT_VerticesSqlDao(db::DB_Connection& connection, std::string query);
	virtual ~PT_VerticesSqlDao();

private:
	/**
	 * Virtual override.
	 * Fills the given outObj with all values contained on Row.
	 * @param result row with data to fill the out object.
	 * @param outObj to fill.
	 */
	void fromRow(db::Row& result, PT_NetworkVertex& outObj);

	/**
	 * Virtual override.
	 * Fills the outParam with all values to insert or update on datasource.
	 * @param data to get values.
	 * @param outParams to put the data parameters.
	 * @param update tells if operation is an Update or Insert.
	 */
	void toRow(PT_NetworkVertex& data, db::Parameters& outParams, bool update);
};

/**
 * DAO for PT edge table
 *
 * \author Prabhuraj
 */
class Pt_EdgesSqlDao: public db::SqlAbstractDao<PT_NetworkEdge>
{
public:
	Pt_EdgesSqlDao(db::DB_Connection& connection, std::string query);
	virtual ~Pt_EdgesSqlDao();

private:
	/**
	 * Virtual override.
	 * Fills the given outObj with all values contained on Row.
	 * @param result row with data to fill the out object.
	 * @param outObj to fill.
	 */
	void fromRow(db::Row& result, PT_NetworkEdge& outObj);

	/**
	 * Virtual override.
	 * Fills the outParam with all values to insert or update on datasource.
	 * @param data to get values.
	 * @param outParams to put the data parameters.
	 * @param update tells if operation is an Update or Insert.
	 */
	void toRow(PT_NetworkEdge& data, db::Parameters& outParams, bool update);
};
} // End sim::mob namespace
