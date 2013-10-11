//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   DBConnection.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 23, 2013, 3:34 PM
 */
#pragma once
#include "soci.h"
#include "DatabaseConfig.hpp"
namespace sim_mob {
    
    namespace db {

        enum BackendType {
            POSTGRES,
            MYSQL, //not implemented
            ORACLE, //not implemented
            SQL_SERVER//not implemented
        };

        /** 
         * Class that represents an Database connection.
         */
        class DBConnection {
        public:
            DBConnection(BackendType type, const DatabaseConfig& config);
            virtual ~DBConnection();

            /**
             * Connects to the database.
             * @return true if the connection was established.
             */
            bool Connect();

            /**
             * Disconnects from the database.
             * @return true if the connection was closed.
             */
            bool Disconnect();

            /**
             * Tells if this instance is connected with database.
             * @return true if connection is open, false otherwise.
             */
            bool IsConnected() const;

            /**
             * Gets the current SOCI session.
             * @return session instance reference.
             */
            soci::session& GetSession();

        private:
            soci::session currentSession;
            std::string connectionStr;
            BackendType type;
            volatile bool connected;
        };
    }
}

