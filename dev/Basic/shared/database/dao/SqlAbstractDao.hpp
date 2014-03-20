//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   AbstractDao.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 23, 2013, 4:29 PM
 */
#pragma once
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>
#include "database/DB_Connection.hpp"
#include "util/LangHelpers.hpp"
#include "soci.h"
#include "I_Dao.h"

namespace {

typedef soci::details::use_type_ptr UseTypePtr;

/**
 * Visitor that will convert an variant value into details::use_type_ptr.
 */
class UsePtrConverter: public boost::static_visitor<UseTypePtr> {
public:
	// visitor interfaces

	template<typename T>
	UseTypePtr operator()(const T& val) const {
		return soci::use(val);
	}
};
// POSTGRES dependent. Needs to be fixed.
const std::string DB_RETURNING_CLAUSE = "RETURNING";
const std::string DB_RETURNING_ALL_CLAUSE = " " + DB_RETURNING_CLAUSE + " * ";
}

namespace sim_mob {

namespace db {
typedef soci::row Row;
typedef soci::rowset<Row> ResultSet;

/**
 * Represents an Abstract implementation of Data Access Object for
 * SQL databases to a given template entity.
 *
 * Example:
 *     DB Table: Person
 *     Entity on System: Person
 *     Dao on System: PersonDao : AbstractDao<Person>
 *
 * Each entity *must* provide two main things:
 * Mandatory:
 *  - Empty constructor.
 *
 * Optional:
 *  - Operator<< defined
 *  - Friendship to the concrete Dao. (nice to have)
 *
 * Each DAO implementation *must* provide the following things:
 * Mandatory:
 *  - override fromRow() to convert SELECT statements to Entities.
 *  - override toRow() to convert data for CRUD statements.
 *  - Default queries:
 *      defaultQueries[INSERT] = "INSERT.....";
 *      defaultQueries[UPDATE] = "UPDATE.....";
 *      defaultQueries[DELETE] = "DELETE.....";
 *      defaultQueries[GET_ALL] = "SELECT * FROM.....";
 *      defaultQueries[GET_BY_ID] = SELECT * FROM..WHERE..";
 * NOTE: all parameters values for prepared statements are used like
 *       "field = :myfield"
 *
 *
 * Attention: The given connection is not managed by DAO implementation.
 * This implementation is not thread-safe.
 *
 */
template<typename T> class SqlAbstractDao: public I_Dao<T> {
public:

	SqlAbstractDao(DB_Connection& connection, const std::string& tableName,
			const std::string& insertQuery, const std::string& updateQuery,
			const std::string& deleteQuery, const std::string& getAllQuery,
			const std::string& getByIdQuery) :
			connection(connection), tableName(tableName) {
		defaultQueries[INSERT] = insertQuery;
		defaultQueries[UPDATE] = updateQuery;
		defaultQueries[DELETE] = deleteQuery;
		defaultQueries[GET_ALL] = getAllQuery;
		defaultQueries[GET_BY_ID] = getByIdQuery;
	}

	virtual ~SqlAbstractDao() {
	}

	virtual T& insert(T& entity, bool returning = false) {
		if (isConnected()) {
			// Get data to insert.
			Parameters params;
			toRow(entity, params, false);

			Transaction tr(connection.getSession<soci::session>());
			Statement query(connection.getSession<soci::session>());
			//append returning clause.
			//Attention: this is only prepared for POSTGRES.
			std::string upperQuery = boost::to_upper_copy(
					defaultQueries[INSERT]);

			if (returning) {
				size_t found = upperQuery.rfind(DB_RETURNING_CLAUSE);
				if (found == std::string::npos) {
					upperQuery += DB_RETURNING_ALL_CLAUSE;
				}
			}

			// prepare statement.
			prepareStatement(upperQuery, params, query);

			//TODO: POSTGRES ONLY for now
			//execute and return data if (RETURNING clause is defined)
			ResultSet rs(query);

			if (returning) {
				ResultSet::const_iterator it = rs.begin();
				if (it != rs.end()) {
					fromRow((*it), entity);
				}
			}
			tr.commit();
		}
		return entity;
	}

	virtual bool update(T& entity) {
		if (isConnected()) {
			Transaction tr(connection.getSession<soci::session>());
			Statement query(connection.getSession<soci::session>());
			// Get data to insert.
			Parameters params;
			toRow(entity, params, true);
			// prepare statement.
			prepareStatement(defaultQueries[UPDATE], params, query);
			ResultSet rs(query);
			tr.commit();
			return true;
		}
		return false;
	}

	virtual bool erase(const Parameters& params) {
		if (isConnected()) {
			Transaction tr(connection.getSession<soci::session>());
			Statement query(connection.getSession<soci::session>());
			// prepare statement.
			prepareStatement(defaultQueries[DELETE], params, query);
			//execute query.
			ResultSet rs(query);
			tr.commit();
			return true;
		}
		return false;
	}

	virtual bool getById(const Parameters& ids, T& outParam) {
		return getByValues(defaultQueries[GET_BY_ID], ids, outParam);
	}

	virtual bool getAll(std::vector<T>& outList) {
		return getByValues(defaultQueries[GET_ALL], EMPTY_PARAMS, outList);
	}

	virtual bool getAll(std::vector<T*>& outList) {
		return getByValues(defaultQueries[GET_ALL], EMPTY_PARAMS, outList);
	}

	template<typename K, typename F>
	bool getAll(boost::unordered_map<K, T>& outMap, F getter) {
		return getByValues(defaultQueries[GET_ALL], EMPTY_PARAMS, outMap,
				getter);
	}

	template<typename K, typename F>
	bool getAll(boost::unordered_map<K, T*>& outMap, F getter) {
		return getByValues(defaultQueries[GET_ALL], EMPTY_PARAMS, outMap,
				getter);
	}

protected:
	// Protected types

	/**
	 * Converts a given row into a T type.
	 * @param result result row.
	 * @param outParam (Out parameter) to receive data from row.
	 */
	virtual void fromRow(Row& result, T& outParam) {
	}

	/**
	 * Converts the given T data into a Parameter list to create a row.
	 * @param data to create a new row.
	 * @param outParams Parameter list that will receive the data.
	 * @param update tells you if it is an update or an insert.
	 */
	virtual void toRow(T& data, Parameters& outParams, bool update) {
	}

	/**
	 * Enum for default queries.
	 */
	enum DefaultQuery {
		INSERT = 0, UPDATE, DELETE, GET_ALL, GET_BY_ID, NUM_QUERIES
	};

	typedef soci::details::prepare_temp_type Statement;
	typedef soci::transaction Transaction;

protected:
	// methods

	/**
	 * Tells we DAO has connection to the database.
	 * @return
	 */
	bool isConnected() {
		return (connection.isConnected());
	}

	/**
	 * Helper function that allows get a list of
	 * Entity objects (vector<V>) by given params.
	 * The output will be assigned on outParam using the *fromRow*.
	 * @param queryStr query string.
	 * @param params to filter the query (to put on Where clause).
	 * @param outParam to fill with retrieved objects.
	 * @param getter to get the object key to insert on map.
	 * @return true if some value was returned, false otherwise.
	 */
	template<typename V, typename F>
	bool getByValues(const std::string& queryStr, const Parameters& params,
			V& outParam, F getter) {
		bool hasValues = false;
		if (isConnected()) {
			Statement query(connection.getSession<soci::session>());
			prepareStatement(queryStr, params, query);
			ResultSet rs(query);
			ResultSet::const_iterator it = rs.begin();
			for (it; it != rs.end(); ++it) {
				appendRow((*it), outParam, getter);
				hasValues = true;
			}
		}
		return hasValues;
	}

	/**
	 * Helper function that allows get a list of
	 * Entity objects (vector<V>) by given params.
	 * The output will be assigned on outParam using the *fromRow*.
	 * @param queryStr query string.
	 * @param params to filter the query (to put on Where clause).
	 * @param outParam to fill with retrieved objects.
	 * @return true if some value was returned, false otherwise.
	 */
	template<typename V>
	bool getByValues(const std::string& queryStr, const Parameters& params,
			V& outParam) {
		bool hasValues = false;
		if (isConnected()) {
			Statement query(connection.getSession<soci::session>());
			prepareStatement(queryStr, params, query);
			ResultSet rs(query);
			ResultSet::const_iterator it = rs.begin();
			for (it; it != rs.end(); ++it) {
				appendRow((*it), outParam);
				hasValues = true;
			}
		}
		return hasValues;
	}

	/**
	 * Helper function to prepare the given statement.
	 * @param queryStr query string.
	 * @param params to put on the statement.
	 * @param outParam out statement.
	 */
	void prepareStatement(const std::string& queryStr, const Parameters& params,
			Statement& outParam) {
		outParam << queryStr;
		Parameters::const_iterator it = params.begin();
		for (it; it != params.end(); ++it) {
			outParam, boost::apply_visitor(UsePtrConverter(), *it);
		}
	}

private:

	/**
	 * Append row to the given object.
	 * @param row to create the object.
	 * @param list to fill.
	 */
	void appendRow(Row& row, T& obj) {
		fromRow(row, obj);
	}

	/**
	 * Append row using dynamic memory.
	 * @param row to create the object.
	 * @param list to fill.
	 */
	void appendRow(Row& row, std::vector<T*>& list) {
		T* model = new T();
		fromRow(row, *model);
		list.push_back(model);
	}

	/**
	 * Append row using automatic memory.
	 * @param row to create the object.
	 * @param list to fill.
	 */
	void appendRow(Row& row, std::vector<T>& list) {
		T model;
		fromRow(row, model);
		list.push_back(model);
	}

	/**
	 * Append row using automatic memory.
	 * @param row to create the object.
	 * @param map to fill.
	 * @param getter to get the key.
	 */
	template<typename K, typename F>
	void appendRow(Row& row, boost::unordered_map<K, T>& map, F getter) {
		T model;
		fromRow(row, model);
		map.insert(std::make_pair(((model).*getter)(), model));
	}

	/**
	 * Append row using dynamic memory.
	 * @param row to create the object.
	 * @param map to fill.
	 * @param getter to get the key.
	 */
	template<typename K, typename F>
	void appendRow(Row& row, boost::unordered_map<K, T*>& map, F getter) {
		T* model = new T();
		fromRow(row, *model);
		map.insert(std::make_pair(((model).*getter)(), model));
	}

protected:
	DB_Connection& connection;
	std::string tableName;
	std::string defaultQueries[NUM_QUERIES + 1];
};
}
}
