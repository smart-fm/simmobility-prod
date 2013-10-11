/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DatabaseConfig.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on October 11, 2013, 11:26 AM
 */

#include "DatabaseConfig.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace sim_mob;
using namespace sim_mob::db;
using std::string;

DatabaseConfig::DatabaseConfig(const string& file) : file(file) {
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(file, pt);
    this->host = pt.get<string>("database.host");
    this->port = pt.get<int>("database.port");
    this->username = pt.get<string>("database.username");
    this->password = pt.get<string>("database.password");
    this->databaseName = pt.get<string>("database.dbname");
}

DatabaseConfig::DatabaseConfig(const DatabaseConfig& orig) {
    this->host = orig.host;
    this->port = orig.port;
    this->username = orig.username;
    this->password = orig.password;
    this->databaseName = orig.databaseName;
}

DatabaseConfig::~DatabaseConfig() {
}

std::string DatabaseConfig::GetDatabaseName() const {
    return databaseName;
}

std::string DatabaseConfig::GetPassword() const {
    return password;
}

std::string DatabaseConfig::GetUsername() const {
    return username;
}

int DatabaseConfig::GetPort() const {
    return port;
}

std::string DatabaseConfig::GetHost() const {
    return host;
}
