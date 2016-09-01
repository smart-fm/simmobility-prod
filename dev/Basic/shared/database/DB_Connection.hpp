//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "DB_Config.hpp"

namespace sim_mob
{

namespace db
{

enum BackendType
{
	POSTGRES, MYSQL, //not implemented
	ORACLE, //not implemented
	SQL_SERVER, //not implemented
	MONGO_DB
};

/**
 * Class that represents an Database connection.
 *
 * \author Pedro Gandola
 */
class DB_Connection
{

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

	std::string getConnectionStr();

private:
	void* currentSession;
	std::string connectionStr;
	BackendType type;
	volatile bool connected;
};
}
}

