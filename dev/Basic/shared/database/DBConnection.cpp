/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DBConnection.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 23, 2013, 3:34 PM
 */

#include "DBConnection.hpp"
#include "util/LangHelpers.hpp"
#include "soci-postgresql.h"

using namespace sim_mob::db;
using std::string;
using soci::postgresql;
using soci::session;

DBConnection::DBConnection(BackendType type, const string& connectionStr) :
connectionStr(connectionStr), currentSession(), type(type), connected(false) {
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
