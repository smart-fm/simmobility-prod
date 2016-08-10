//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <boost/format.hpp>
#include <soci/postgresql/soci-postgresql.h>
#include <soci/soci.h>
#include "DB_Connection.hpp"
#include "util/LangHelpers.hpp"

using namespace sim_mob::db;
using std::string;
using soci::postgresql;
using soci::session;

namespace
{
const std::string PGSQL_CONNSTR_FORMAT = "host=%1% "
										 "port=%2% "
										 "user=%3% "
										 "password=%4% "
										 "dbname=%5%";

/**
 * Class that holds the session object.
 *
 * \author Pedro Gandola
 */
template<typename T>
class DB_Session
{
public:

	DB_Session()
	{
	}

	virtual ~DB_Session()
	{
	}

	T& getSession()
	{
		return session;
	}
	T session;
};

/**
 * Soci session holder.
 */
typedef DB_Session<soci::session> SociSessionImpl;
}

DB_Connection::DB_Connection(BackendType type, const DB_Config& config) :
		currentSession(nullptr), type(type), connected(false)
{
	switch (type)
	{
	case POSTGRES:
	{
		const std::string PGSQL_CONNSTR_FORMAT2 = "host=%1% port=%2% user=%3% password=%4% dbname=%5%";
		boost::format fmtr = boost::format(PGSQL_CONNSTR_FORMAT2);
		fmtr % config.getHost() % config.getPort() % config.getUsername() % config.getPassword() % config.getDatabaseName();
		connectionStr = fmtr.str();
		break;
	}
	default:
		break;
	}
}

DB_Connection::~DB_Connection()
{
	disconnect();
}

bool DB_Connection::connect()
{
	if (!connected)
	{
		switch (type)
		{
		case POSTGRES:
		{
			currentSession = new SociSessionImpl();
			getSession<soci::session>().open(postgresql, connectionStr);
			connected = true;
			break;
		}
		default:
			break;
		}
	}
	return connected;
}

bool DB_Connection::disconnect()
{
	if (connected)
	{
		switch (type)
		{
		case POSTGRES:
		{
			getSession<soci::session>().close();
			delete (SociSessionImpl*) currentSession;
			break;
		}
		default:
			break;
		}
		connected = false;
	}
	return !connected;
}

bool DB_Connection::isConnected() const
{
	return connected;
}

template<typename T> T& DB_Connection::getSession()
{
	return ((DB_Session<T>*) (currentSession))->getSession();
}

template soci::session& DB_Connection::getSession<soci::session>();
