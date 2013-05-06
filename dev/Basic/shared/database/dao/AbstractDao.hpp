/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   AbstractDao.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 23, 2013, 4:29 PM
 */
#pragma once
#include <vector>
#include "database/DBConnection.hpp"
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>
#include "util/LangHelpers.hpp"

using std::string;
using std::vector;
using namespace boost;

namespace {
    typedef soci::details::use_type_ptr UseTypePtr;

    /**
     * Visitor that will convert an variant value into details::use_type_ptr.
     */
    class UsePtrConverter : public static_visitor<UseTypePtr> {
    public: // visitor interfaces

        template <typename T>
        UseTypePtr operator()(const T& val) const {
            return use(val);
        }
    };
    // POSTGRES dependent. Needs to be fixed.
    const string DB_RETURNING_CLAUSE = "RETURNING";
    const string DB_RETURNING_ALL_CLAUSE = " " + DB_RETURNING_CLAUSE + " * ";
}

namespace sim_mob {

    namespace dao {
        typedef boost::variant<int, string, double, long long, unsigned long> Parameter;
        typedef vector<Parameter> Parameters;
        typedef row Row;
        typedef rowset<Row> ResultSet;
    }
    using namespace sim_mob::dao;

#define DAO_DECLARE_CALLBACKS(modelType)\
        typedef void (AbstractDao<modelType>::*modelType##FromCallback)(sim_mob::dao::Row& result, modelType& outParam);\
        typedef void (AbstractDao<modelType>::*modelType##ToCallback)(modelType& data, dao::Parameters& outParam, bool update);

#define DAO_FROM_ROW_CALLBACK_HANDLER(modelType, func) (AbstractDao<modelType>::FromRowCallback)(modelType##FromCallback) &func
#define DAO_TO_ROW_CALLBACK_HANDLER(modelType, func) (AbstractDao<modelType>::ToRowCallback)(modelType##ToCallback) &func

    /**
     * Represents an Abstract Data Access Object to a given template entity.
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
     *  - fromRowCallback function implemented
     *  - toRowCallback function implemented
     *  - Default queries:
     *      defaultQueries[INSERT] = "INSERT.....";
     *      defaultQueries[UPDATE] = "UPDATE.....";
     *      defaultQueries[DELETE] = "DELETE.....";
     *      defaultQueries[GET_ALL] = "SELECT * FROM.....";
     *      defaultQueries[GET_BY_ID] = SELECT * FROM..WHERE..";
     * NOTE: all parameters values for prepared statements are used like "field = :myfield"
     * 
     * Optional:
     *  - Operator<< defined
     *  - Friendship to the concrete Dao. (nice to have)
     * 
     * Attention: The given connection is not managed by the DAO implementation.
     * 
     */
    template <typename T> class AbstractDao {
    public:

        AbstractDao(DBConnection* connection, const string& tableName,
                const string& insertQuery, const string& updateQuery,
                const string& deleteQuery, const string& getAllQuery,
                const string& getByIdQuery)
        : connection(connection), tableName(tableName),
        fromRowCallback(nullptr), toRowCallback(nullptr) {
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
         * Inserts the given entity into the datasource.
         * @param entity to insert.
         * @return true if the transaction was commited with success, false otherwise.
         */
        virtual T& Insert(T& entity) {
            if (toRowCallback && IsConnected()) {
                Transaction tr(connection->GetSession());
                Statement query(connection->GetSession());
                //append returning clause. 
                //Attention: this is only prepared for POSTGRES.
                string upperQuery = boost::to_upper_copy(defaultQueries[INSERT]);
                int found = upperQuery.rfind(DB_RETURNING_CLAUSE);
                if (found == std::string::npos) {
                    upperQuery += DB_RETURNING_ALL_CLAUSE;
                }
                // Get data to insert.
                Parameters params;
                (this->*(toRowCallback))(entity, params, false);
                // prepare statement.
                PrepareStatement(upperQuery, params, query);
                //execute and return data if (RETURNING clause is well defined)
                ResultSet rs(query);
                ResultSet::const_iterator it = rs.begin();
                if (it != rs.end()) {
                    (this->*(fromRowCallback))((*it), entity);
                }
                tr.commit();
            }
            return entity;
        }

        /**
         * Updates the given entity into the datasource.
         * @param entity to update.
         * @return true if the transaction was commited with success, false otherwise.
         */
        virtual bool Update(T& entity) {
            if (toRowCallback && IsConnected()) {
                Transaction tr(connection->GetSession());
                Statement query(connection->GetSession());
                // Get data to insert.
                Parameters params;
                (this->*(toRowCallback))(entity, params, true);
                // prepare statement.
                PrepareStatement(defaultQueries[UPDATE], params, query);
                ResultSet rs(query);
                tr.commit();
                return true;
            }
            return false;
        }

        /**
         * Deletes all objects filtered by given ids.
         * @param ids to filter.
         * @return true if the transaction was commited with success, false otherwise.
         */
        virtual bool Delete(const Parameters& ids) {
            if (IsConnected()) {
                Transaction tr(connection->GetSession());
                Statement query(connection->GetSession());
                // prepare statement.
                PrepareStatement(defaultQueries[DELETE], ids, query);
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
        virtual bool GetById(const Parameters& ids, T& outParam) {
            return GetByValues(defaultQueries[GET_BY_ID], ids, outParam);
        }

        /**
         * Gets all values from the source and put them on the given list.
         * @param outList to put the retrieved values. 
         * @return true if some values were returned, false otherwise.
         */
        virtual bool GetAll(vector<T>& outList) {
            return GetByValues(defaultQueries[GET_ALL], EMPTY_PARAMS, outList);
        }

    protected: // Protected types

        typedef void (AbstractDao::*FromRowCallback)(Row& result, T& outParam);
        typedef void (AbstractDao::*ToRowCallback)(T& data, Parameters& outParams, bool update);

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
        bool IsConnected() {
            return (connection && connection->IsConnected());
        }

        /**
         * Helper function that allows get a list of Entity objects (vector<T>) by given params. 
         * The output will be assigned on outParam using the *fromRowCallback*.
         * @param queryStr query string.
         * @param params to filter the query (to put on Where clause).
         * @param outParam to fill with retrieved objects.
         * @return true if some value was returned, false otherwise.
         */
        bool GetByValues(const string& queryStr, const Parameters& params, vector<T>& outParam) {
            bool hasValues = false;
            if (fromRowCallback && IsConnected()) {
                Statement query(connection->GetSession());
                PrepareStatement(queryStr, params, query);
                ResultSet rs(query);
                for (ResultSet::const_iterator it = rs.begin(); it != rs.end(); ++it) {
                    T model;
                    (this->*(fromRowCallback))((*it), model);
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
        bool GetByValues(const string& queryStr, const Parameters& params, T& outParam) {
            if (fromRowCallback && IsConnected()) {
                Statement query(connection->GetSession());
                PrepareStatement(queryStr, params, query);
                ResultSet rs(query);
                ResultSet::const_iterator it = rs.begin();
                if (it != rs.end()) {
                    (this->*(fromRowCallback))((*it), outParam);
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
        void PrepareStatement(const string& queryStr, const Parameters& params, Statement& outParam) {
            outParam << queryStr;
            for (Parameters::const_iterator it = params.begin(); it != params.end(); ++it) {
                outParam, boost::apply_visitor(UsePtrConverter(), *it);
            }
        }

    protected:
        DBConnection* connection;
        string tableName;
        string defaultQueries[5];
        FromRowCallback fromRowCallback;
        ToRowCallback toRowCallback;
        const Parameters EMPTY_PARAMS;
    };
}