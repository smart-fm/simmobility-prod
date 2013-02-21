#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include <iostream>
#include <sstream>

using sim_mob::DatabaseConnection;
using std::string;
using std::pair;

void sim_mob::conf::db_connection_pimpl::pre ()
{
}

pair<string, DatabaseConnection> sim_mob::conf::db_connection_pimpl::post_db_connection ()
{
	//Some input validation
	if (dbcType!="postgres") {
		throw std::runtime_error("Only postgres database connections supported for now.");
	}
	if (dbcParams.count("host")==0 || dbcParams.count("port")==0 || dbcParams.count("dbname")==0 || dbcParams.count("user")==0 || dbcParams.count("password")==0) {
		throw std::runtime_error("Database connection missing some required parameters.");
	}

	//Create a return type.
	DatabaseConnection res(dbcID);
	res.dbName = dbcParams.find("dbname")->second;
	res.host = dbcParams.find("host")->second;
	res.password = Password(dbcParams.find("password")->second);
	res.port = boost::lexical_cast<int>(dbcParams.find("port")->second);
	res.user = dbcParams.find("user")->second;

	//Return it
	return std::make_pair(res.getId(), res);
}

void sim_mob::conf::db_connection_pimpl::param (const std::pair<std::string, std::string>& value)
{
	dbcParams[value.first] = value.second;
}

void sim_mob::conf::db_connection_pimpl::id (const ::std::string& id)
{
	dbcID = id;
}

void sim_mob::conf::db_connection_pimpl::dbtype (const ::std::string& dbtype)
{
	dbcType = dbtype;
}

