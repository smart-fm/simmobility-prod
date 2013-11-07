//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   DBConnection.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 23, 2013, 3:34 PM
 */

#include <boost/format.hpp>
#include "DBConnection.hpp"
#include "util/LangHelpers.hpp"
#include "soci-postgresql.h"

using namespace sim_mob::db;
using std::string;
using soci::postgresql;
using soci::session;

namespace {
    const std::string PGSQL_CONNSTR_FORMAT = "host=%1% port=%2% user=%3% password=%4% dbname=%5%";
}

DBConnection::DBConnection(BackendType type, const DatabaseConfig& config) 
: currentSession(), type(type), connected(false) {
    switch (type) {
        case POSTGRES:
        {
            boost::format fmtr = boost::format(PGSQL_CONNSTR_FORMAT);
            fmtr % config.getHost() % config.getPort() % config.getUsername() % config.getPassword() % config.getDatabaseName();
            connectionStr = fmtr.str();
            break;
        }
        default:break;
    }
}

DBConnection::~DBConnection() {
    Disconnect();
}

bool DBConnection::Connect() {
    if (!connected) {
        switch (type) {
            case POSTGRES:
            {
                currentSession.open(postgresql, connectionStr);
                connected = true;
                break;
            }
            default:break;
        }
    }
    return connected;
}

bool DBConnection::Disconnect() {
    if (connected) {
        currentSession.close();
        connected = false;
    }
    return !connected;
}

bool DBConnection::IsConnected() const {
    return connected;
}

session& DBConnection::GetSession() {
    return currentSession;
}
