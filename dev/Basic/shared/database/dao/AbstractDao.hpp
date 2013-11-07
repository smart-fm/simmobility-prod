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
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>
#include "database/DBConnection.hpp"
#include "util/LangHelpers.hpp"

namespace {

    typedef soci::details::use_type_ptr UseTypePtr;

    /**
     * Visitor that will convert an variant value into details::use_type_ptr.
     */
    class UsePtrConverter : public boost::static_visitor<UseTypePtr> {
    public: // visitor interfaces

        template <typename T>
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

        typedef boost::variant<int, std::string, double, long long, unsigned long> Parameter;
        typedef std::vector<Parameter> Parameters;
        typedef soci::row Row;
        typedef soci::rowset<Row> ResultSet;
        static const Parameters EMPTY_PARAMS;

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
        template <typename T> class AbstractDao {
        public:

            AbstractDao(DBConnection* connection, const std::string& tableName,
                    const std::string& insertQuery, const std::string& updateQuery,
                    const std::string& deleteQuery, const std::string& getAllQuery,
                    const std::string& getByIdQuery)
            : connection(connection), tableName(tableName) {
                defaultQueries[INSERT] = insertQuery;
                defaultQueries[UPDATE] = updateQuery;
                defaultQueries[DELETE] = deleteQuery;
                defaultQueries[GET_ALL] = getAllQuery;
                defaultQueries[GET_BY_ID] = getByIdQuery;
            }

            virtual ~AbstractDao() {
                connection = nullptr;
            }

            /**
             * Inserts the given entity into the data source.
             * @param entity to insert.
             * @return true if the transaction was committed with success,
             *         false otherwise.
             */
            virtual T& insert(T& entity) {
                if (isConnected()) {
                    Transaction tr(connection->GetSession());
                    Statement query(connection->GetSession());
                    //append returning clause. 
                    //Attention: this is only prepared for POSTGRES.
                    std::string upperQuery = boost::to_upper_copy(defaultQueries[INSERT]);
                    size_t found = upperQuery.rfind(DB_RETURNING_CLAUSE);
                    if (found == std::string::npos) {
                        upperQuery += DB_RETURNING_ALL_CLAUSE;
                    }
                    // Get data to insert.
                    Parameters params;
                    toRow(entity, params, false);
                    // prepare statement.
                    prepareStatement(upperQuery, params, query);
                    //TODO: POSTGRES ONLY for now
                    //execute and return data if (RETURNING clause is defined)
                    ResultSet rs(query);
                    ResultSet::const_iterator it = rs.begin();
                    if (it != rs.end()) {
                        fromRow((*it), entity);
                    }
                    tr.commit();
                }
                return entity;
            }

            /**
             * Updates the given entity into the data source.
             * @param entity to update.
             * @return true if the transaction was committed with success, 
             *         false otherwise.
             */
            virtual bool update(T& entity) {
                if (isConnected()) {
                    Transaction tr(connection->GetSession());
                    Statement query(connection->GetSession());
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

            /**
             * Deletes all objects filtered by given params.
             * @param params to filter.
             * @return true if the transaction was committed with success, 
             *         false otherwise.
             */
            virtual bool erase(const Parameters& params) {
                if (isConnected()) {
                    Transaction tr(connection->GetSession());
                    Statement query(connection->GetSession());
                    // prepare statement.
                    prepareStatement(defaultQueries[DELETE], params, query);
                    //execute query.
                    ResultSet rs(query);
                    tr.commit();
                    return true;
                }
                return false;
            }

            /**
             * Gets a single value filtered by given ids. 
             * @param ids to filter.
             * @param outParam to put the value
             * @return true if a value was returned, false otherwise.
             */
            virtual bool getById(const Parameters& ids, T& outParam) {
                return getByValues(defaultQueries[GET_BY_ID], ids, outParam);
            }

            /**
             * Gets all values from the source and put them on the given list.
             * @param outList to put the retrieved values. 
             * @return true if some values were returned, false otherwise.
             */
            virtual bool getAll(std::vector<T>& outList) {
                return getByValues(defaultQueries[GET_ALL], EMPTY_PARAMS, outList);
            }

        protected: // Protected types

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
                INSERT = 0,
                UPDATE = 1,
                DELETE = 2,
                GET_ALL = 3,
                GET_BY_ID = 4
            };

            typedef soci::details::prepare_temp_type Statement;
            typedef soci::transaction Transaction;

        protected: // methods

            /**
             * Tells we DAO has connection to the database.
             * @return 
             */
            bool isConnected() {
                return (connection && connection->IsConnected());
            }

            /**
             * Helper function that allows get a list of 
             * Entity objects (vector<T>) by given params. 
             * The output will be assigned on outParam using the *fromRowCallback*.
             * @param queryStr query string.
             * @param params to filter the query (to put on Where clause).
             * @param outParam to fill with retrieved objects.
             * @return true if some value was returned, false otherwise.
             */
            bool getByValues(const std::string& queryStr,
                    const Parameters& params, std::vector<T>& outParam) {
                bool hasValues = false;
                if (isConnected()) {
                    Statement query(connection->GetSession());
                    prepareStatement(queryStr, params, query);
                    ResultSet rs(query);
                    ResultSet::const_iterator it = rs.begin();
                    for (it; it != rs.end(); ++it) {
                        T model;
                        fromRow((*it), model);
                        outParam.push_back(model);
                        hasValues = true;
                    }
                }
                return hasValues;
            }

            /**
             * Helper function that allows get a Entity <T> by given params. 
             * The output will be assigned on outParam using the *fromRowCallback*.
             * @param queryStr query string.
             * @param params to filter the query (to put on Where clause).
             * @param outParam to fill with retrieved object.
             * @return true if a value was returned, false otherwise.
             */
            bool getByValues(const std::string& queryStr,
                    const Parameters& params, T& outParam) {
                if (isConnected()) {
                    Statement query(connection->GetSession());
                    prepareStatement(queryStr, params, query);
                    ResultSet rs(query);
                    ResultSet::const_iterator it = rs.begin();
                    if (it != rs.end()) {
                        fromRow((*it), outParam);
                        return true;
                    }
                }
                return false;
            }

            /**
             * Helper function to prepare the given statement.
             * @param queryStr query string.
             * @param params to put on the statement.
             * @param outParam out statement.
             */
            void prepareStatement(const std::string& queryStr,
                    const Parameters& params, Statement& outParam) {
                outParam << queryStr;
                Parameters::const_iterator it = params.begin();
                for (it; it != params.end(); ++it) {
                    outParam, boost::apply_visitor(UsePtrConverter(), *it);
                }
            }

        protected:
            DBConnection* connection;
            std::string tableName;
            std::string defaultQueries[5];
        };
    }
}