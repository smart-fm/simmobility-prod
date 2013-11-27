//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   DB_Connection.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 23, 2013, 3:34 PM
 */

#pragma once
#include "DB_Config.hpp"

namespace sim_mob {

    namespace db {

        enum BackendType {
            POSTGRES,
            MYSQL, //not implemented
            ORACLE, //not implemented
            SQL_SERVER//not implemented
        };

        /*

        /** 
         * Class that represents an Database connection.
         */
        class DB_Connection {

        public:
            DB_Connection(BackendType type, const DB_Config& config);
            virtual ~DB_Connection();

            /**
             * Connects to the database.
             * @return true if the connection was established.
             */
            bool connect();

            /**
             * Disconnects from the database.
             * @return true if the connection was closed.
             */
            bool disconnect();

            /**
             * Tells if this instance is connected with database.
             * @return true if connection is open, false otherwise.
             */
            bool isConnected() const;

            /**
             * Gets the current SOCI session.
             * @return session instance reference.
             */
            template<typename T> T& getSession(); 

        private:
            void* currentSession;
            std::string connectionStr;
            BackendType type;
            volatile bool connected;
        };
    }
}

