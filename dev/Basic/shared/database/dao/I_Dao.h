//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   I_Dao.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on November 27, 2013, 4:29 PM
 */
#pragma once
#include<vector>
#include<string>
#include <boost/variant.hpp>

namespace sim_mob {

    namespace db {

        typedef boost::variant<int, std::string, double, long long, 
                unsigned long> Parameter;
        typedef std::vector<Parameter> Parameters;
        static const Parameters EMPTY_PARAMS;

        /**
         * Represents an Interface for DAO (Data Access Object) implementations.
         */
        template<typename T>
        class I_Dao {
        public:

            virtual ~I_Dao() {
            }

            /**
             * Inserts the given entity into the data source.
             * @param entity to insert.
             * @return true if the transaction was committed with success,
             *         false otherwise.
             */
            virtual T& insert(T& entity) = 0;

            /**
             * Updates the given entity into the data source.
             * @param entity to update.
             * @return true if the transaction was committed with success, 
             *         false otherwise.
             */
            virtual bool update(T& entity) = 0;

            /**
             * Deletes all objects filtered by given params.
             * @param params to filter.
             * @return true if the transaction was committed with success, 
             *         false otherwise.
             */
            virtual bool erase(const Parameters& params) = 0;

            /**
             * Gets a single value filtered by given ids. 
             * @param ids to filter.
             * @param outParam to put the value
             * @return true if a value was returned, false otherwise.
             */
            virtual bool getById(const Parameters& ids, T& outParam) = 0;

            /**
             * Gets all values from the source and put them on the given list.
             * @param outList to put the retrieved values. 
             * @return true if some values were returned, false otherwise.
             */
            virtual bool getAll(std::vector<T>& outList) = 0;
        };
    }
}
