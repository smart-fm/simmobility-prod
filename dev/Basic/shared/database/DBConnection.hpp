/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DBConnection.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 23, 2013, 3:34 PM
 */
#pragma once
#include "soci.h"

using std::string;
using namespace soci;

namespace sim_mob {

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
        DBConnection(BackendType type, const string& connectionStr);
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
        session& GetSession();

    private:
        session currentSession;
        string connectionStr;
        BackendType type;
        volatile bool connected;
    };
}

