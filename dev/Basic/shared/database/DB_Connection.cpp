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
#include "mongo/client/dbclient.h"
#include "soci.h"
#include "soci-postgresql.h"
#include "util/LangHelpers.hpp"


using namespace sim_mob::db;
using namespace mongo;
using std::string;
using soci::postgresql;
using soci::session;

namespace {
    const std::string PGSQL_CONNSTR_FORMAT = "host=%1% "
                                             "port=%2% "
                                             "user=%3% "
                                             "password=%4% " 
                                             "dbname=%5%";

    /**
     * Class that holds the session object.
     */
    template<typename T>
    class DB_Session{
    public:

        DB_Session() {
        }

        virtual ~DB_Session() {
        }

        T& getSession() {
            return session;
        }
        T session;
    };
    
    /**
     * Soci session holder. 
     */
    typedef DB_Session<soci::session> SociSessionImpl;

    /**
     * Mongo session holder
     */
    typedef DB_Session<mongo::DBClientConnection> MongoSessionImpl;
}


DB_Connection::DB_Connection(BackendType type, const DB_Config& config)
: currentSession(nullptr), type(type), connected(false) {
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
        	connectionStr = config.getHost();
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
                currentSession = new SociSessionImpl();
                getSession<soci::session>().open(postgresql, connectionStr);
                connected = true;
                break;
            }
            case MONGO_DB:
            {
            	currentSession = new MongoSessionImpl();
            	getSession<mongo::DBClientConnection>().connect(connectionStr);
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
                getSession<soci::session>().close();
                delete (SociSessionImpl*)currentSession;
                break;
            }
            case MONGO_DB:
            {
            	// No need to explicitly close the connection. Just delete.
            	delete (MongoSessionImpl*)currentSession;
            	break;
            }
            default:break;
        }
        connected = false;
    }
    return !connected;
}

bool DB_Connection::isConnected() const {
    return connected;
}

template<typename T> T& DB_Connection::getSession()
{
    return ((DB_Session<T>*)(currentSession))->getSession();
}

template soci::session& DB_Connection::getSession<soci::session>();
template mongo::DBClientConnection& DB_Connection::getSession<mongo::DBClientConnection>();
