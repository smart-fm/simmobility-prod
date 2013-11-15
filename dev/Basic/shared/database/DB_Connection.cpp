//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   DB_Connection.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 23, 2013, 3:34 PM
 */

#include <boost/format.hpp>
#include "DB_Connection.hpp"
#include "util/LangHelpers.hpp"
#include "soci-postgresql.h"
#include "mongo/client/dbclient.h"

using namespace sim_mob::db;
using std::string;
using soci::postgresql;
using soci::session;

namespace {
    const std::string PGSQL_CONNSTR_FORMAT = "host=%1% "
                                             "port=%2% "
                                             "user=%3% "
                                             "password=%4% " 
                                             "dbname=%5%";
}

DB_Connection::DB_Connection(BackendType type, const DB_Config& config)
: currentSession(), type(type), connected(false) {
    switch (type) {
        case POSTGRES:
        {
            boost::format fmtr = boost::format(PGSQL_CONNSTR_FORMAT);
            fmtr % config.getHost() % config.getPort() % config.getUsername() 
                 % config.getPassword() % config.getDatabaseName();
            connectionStr = fmtr.str();
            break;
        }
        case MONGO_DB:
        {
        	connectionStr = config.GetHost();
        	break;
        }
        default:break;
    }
}

DB_Connection::~DB_Connection() {
    disconnect();
}

bool DB_Connection::connect() {
    if (!connected) {
        switch (type) {
            case POSTGRES:
            {
                currentSession.open(postgresql, connectionStr);
                connected = true;
                break;
            }
            case MONGO_DB:
            {
            	mongoConn.connect(connectionStr);
            	connected = true;
            	break;
            }
            default:break;
        }
    }
    return connected;
}

bool DB_Connection::disconnect() {
    if (connected) {
		switch (type) {
		case POSTGRES:
		{
			currentSession.close();
			break;
		}
		default:
			break;
		}

        connected = false;
    }
    return !connected;
}

bool DB_Connection::isConnected() const {
    return connected;
}

session& DB_Connection::getSession() {
    return currentSession;
}

mongo::DBClientConnection& DB_Connection::getMongoConnection() {
	return mongoConn;
}
