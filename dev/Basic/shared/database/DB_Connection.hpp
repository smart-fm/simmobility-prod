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
#include "soci.h"
#include "DB_Config.hpp"

#include "mongo/client/dbclient.h"
namespace sim_mob {
    
    namespace db {

        enum BackendType {
            POSTGRES,
            MYSQL, //not implemented
            ORACLE, //not implemented
            SQL_SERVER,//not implemented
            MONGO_DB
        };

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
            soci::session& getSession();

            /**
             * Gets the current mongodb connection object.
             * @return DBClientConnection reference
             */
			mongo::DBClientConnection& getMongoConnection();

        private:
            soci::session currentSession;
            mongo::DBClientConnection mongoConn;
            std::string connectionStr;
            BackendType type;
            volatile bool connected;
        };
    }
}

